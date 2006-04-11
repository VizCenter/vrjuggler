/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2005 by Iowa State University
 *
 * Original Authors:
 *   Allen Bierbaum, Christopher Just,
 *   Patrick Hartling, Kevin Meinert,
 *   Carolina Cruz-Neira, Albert Baker
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * -----------------------------------------------------------------
 * File:          $RCSfile$
 * Date modified: $Date$
 * Version:       $Revision$
 * -----------------------------------------------------------------
 *
 *************** <auto-copyright.pl END do not edit this line> ***************/

#include <gadget/Devices/DriverConfig.h>
#include <vpr/Util/Debug.h>
#include <jccl/Config/ConfigElement.h>
#include <gadget/Type/DeviceConstructor.h>
#include <gadget/Util/Debug.h>
#include <gadget/gadgetParam.h>

#include <drivers/Open/LinuxJoydev/LinuxJoydev.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <linux/joystick.h>      // Get the joystick abilities


extern "C"
{

GADGET_DRIVER_EXPORT(vpr::Uint32) getGadgeteerVersion()
{
   return __GADGET_version;
}

GADGET_DRIVER_EXPORT(void) initDevice(gadget::InputManager* inputMgr)
{
   new gadget::DeviceConstructor<gadget::LinuxJoydev>(inputMgr);
}

}

namespace gadget
{


/** Constructor. */
LinuxJoydev::LinuxJoydev()
{;}

/**
 * Destructor.
 *
 * @pre None.
 * @post Shared memory is released.
 */
LinuxJoydev::~LinuxJoydev()
{
}

std::string LinuxJoydev::getElementType()
{
   return "linux_joydev";
}

/**
 *
 */
bool LinuxJoydev::config(jccl::ConfigElementPtr e)
{
   if(! (Input::config(e) && Digital::config(e) && Analog::config(e)))
   {
      return false;
   }

   mJsLabel = e->getName();
   mPortName = e->getProperty<std::string>("port_name");

   unsigned num_axis_buttons = e->getNum("axis_buttons");
   for(unsigned i=0;i<num_axis_buttons;++i)
   {
      unsigned idx = e->getProperty<int>("axis_buttons", i);
      mAxisButtonIndices.push_back(idx);
   }

   return true;
}


/** Begin sampling.
* Connect to the joystick and prepare to read.
*/
bool LinuxJoydev::startSampling()
{
   vprDEBUG(gadgetDBG_INPUT_MGR, vprDBG_CONFIG_STATUS_LVL) << "Opening Linux Joystick driver on port: " << mPortName << std::endl << vprDEBUG_FLUSH;

   mJsFD = open(mPortName.c_str(), O_RDONLY | O_NONBLOCK);   // Open the joystick non-blocking

   if(mJsFD < 0)
   {
      vprDEBUG(gadgetDBG_INPUT_MGR, vprDBG_CRITICAL_LVL) << "ERROR: Failed to open Linux Joystick: " << mPortName << std::endl << vprDEBUG_FLUSH;
      return false;
   }

   // Read in the settings
   int version;
   unsigned char num_axes, num_buttons;
   char js_name[255];

   ioctl(mJsFD, JSIOCGVERSION, &version);

   // Check to see if the version is high enough.
   if (version < 1)
   {
      vprDEBUG(gadgetDBG_INPUT_MGR, vprDBG_CRITICAL_LVL)
         << "ERROR: Linux Joystick API Version is too low.  The LinuxJoydev "
         << "driver requires version 1.0 or greater, but found version \""
         << version << "\"\n" << vprDEBUG_FLUSH;
         return false;
   }
   ioctl(mJsFD, JSIOCGAXES, &num_axes);
   ioctl(mJsFD, JSIOCGBUTTONS, &num_buttons);

   mNumAxes = num_axes;
   mNumButtons = num_buttons;

   // Get joystick name
   if ( ioctl(mJsFD, JSIOCGNAME(sizeof(js_name)), js_name) < 0 )
   {  
      mPhysicalJsName = std::string("Unknown"); 
   }
   else
   {  
      mPhysicalJsName = js_name; 
   }

   // Output joystick description
   vprDEBUG(gadgetDBG_INPUT_MGR, vprDBG_CONFIG_LVL)
         << "  Joystick Label: " << mJsLabel << std::endl
         << "   Joystick Name: " << mPhysicalJsName << std::endl
         << "            Axes: " << mNumAxes << std::endl
         << "         Buttons: " << mNumButtons << std::endl
         << "  Driver version: " << version << std::endl
         << "    Axis buttons: ";

   for(unsigned i=0;i<mAxisButtonIndices.size(); ++i)
   { 
      vprDEBUG_CONTnl(gadgetDBG_INPUT_MGR, vprDBG_CONFIG_LVL)
         << mAxisButtonIndices[i] << " "; 
   }
   vprDEBUG_CONTnl(gadgetDBG_INPUT_MGR, vprDBG_CONFIG_LVL) << std::endl
                                                           << vprDEBUG_FLUSH;

   // Allocate initial device data
   // - By default this will clear them out
   mCurAxes.resize(mNumAxes);
   mCurAxesRanges.resize(mNumAxes, axis_range_t(0,1));             // Initialize ranges to 0,255
   mCurButtons.resize(mNumButtons + mAxisButtonIndices.size());

   // Setup axis as button stuff
   mAxisToButtonIndexLookup.clear();
   mAxisToButtonIndexLookup.resize(mNumAxes, -1);            // Default to -1, meaning no axis button
   for(unsigned i=0;i<mAxisButtonIndices.size(); ++i)       // For each configured axis index
   {
      unsigned virtual_btn_index = (mNumButtons+i); // Index of the virtual button from the axis
      vprASSERT(virtual_btn_index < mCurButtons.size() && "Virtual button index out of range");
      unsigned axis_index = mAxisButtonIndices[i];                      // Index of the axis we are mapping
      mAxisToButtonIndexLookup[axis_index] = int(virtual_btn_index);    // Setup the mapping
   }

   return true;
}

/** Stops sampling.
* Drop connection to joystick and clear everything.
*/
bool LinuxJoydev::stopSampling()
{
   if(mJsFD > 0)
   {
      close(mJsFD);     // Close the joystick device
   }

   return true;
}


/**
 * Updates to the sampled data.
 *
 * @pre None.
 * @post Most recent value is copied over to temp area.
 */
void LinuxJoydev::updateData()
{
   // -- Read in any new pending events
   js_event cur_event;

   // While events pending
   while (read(mJsFD, &cur_event, sizeof(struct js_event)) > 0)
   {
      if(cur_event.type & JS_EVENT_BUTTON)
      {
         //std::cout << "ljs: btn: " << unsigned(cur_event.number) << " val:" << cur_event.value << std::endl;
         unsigned btn_number = unsigned(cur_event.number);
         vprASSERT(btn_number < mCurButtons.size() && "Button out of range");
         mCurButtons[btn_number] = cur_event.value;         // Assign the new button value (0,1)
         mCurButtons[btn_number].setTime();                 // Set timestamp to now
      }
      else if(cur_event.type & JS_EVENT_AXIS)
      {
         //std::cout << "ljs: axis: " << unsigned(cur_event.number) << " val:" << cur_event.value << std::endl;
         unsigned axis_number = unsigned(cur_event.number);
         vprASSERT(axis_number < mCurAxes.size() && "Axis out of range");
         vprASSERT(axis_number < mCurAxesRanges.size() && "Axis out of range");

         float norm_value = 0;
         // Verify range (expand if needed)
         axis_range_t axis_range = mCurAxesRanges[axis_number];
         if(cur_event.value < axis_range.first)
         {
            axis_range.first = cur_event.value;
            mCurAxesRanges[axis_number] = axis_range;
         }
         
         else if(cur_event.value > axis_range.second)
         {
            axis_range.second = cur_event.value;
            mCurAxesRanges[axis_number] = axis_range;
         }

         else
         {
            norm_value = (float(cur_event.value) - axis_range.first)/(axis_range.second-axis_range.first);
         }
         
         mCurAxes[axis_number] = norm_value;
         mCurAxes[axis_number].setTime();

         // Check for axis buttons
         // - If we have a mapping
         // - If axis is gt 0.5, then btn is down
         if(mAxisToButtonIndexLookup[axis_number] != -1)    // If we map to a virtual button
         {
            unsigned vir_btn_index = mAxisToButtonIndexLookup[axis_number];
            vprASSERT(vir_btn_index < mCurButtons.size() && "Virtual button index out of range");
            mCurButtons[vir_btn_index] = ( (norm_value > 0.5f) ? 1 : 0);
            mCurButtons[vir_btn_index].setTime();
         }
      }
   }

   // Check to make sure error was just no-pending events
   if(errno != EAGAIN)
   {
      vprDEBUG(vprDBG_ALL, vprDBG_CRITICAL_LVL) << "ERROR: Error reading linux joystick.\n" << vprDEBUG_FLUSH;
      return;
   }

   addDigitalSample(mCurButtons);
   swapDigitalBuffers();

   addAnalogSample(mCurAxes);
   swapAnalogBuffers();
}


} // End of gadget namespace
