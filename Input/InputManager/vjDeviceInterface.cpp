/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998, 1999, 2000, 2001, 2002
 *   by Iowa State University
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


#include <Input/InputManager/vjDeviceInterface.h>
#include <Kernel/vjKernel.h>
#include <Input/InputManager/vjInputManager.h>


vjDeviceInterface::vjDeviceInterface()
: mProxyIndex(-1), mProxyName("UnInitialized"), mNameSet(false)
{
   vjDeviceInterface::addDevInterface(this);    // Keep referense to the interface
}

vjDeviceInterface::~vjDeviceInterface()
{
   removeDevInterface(this);     // Remove it from the list of active interfaces;
}


void vjDeviceInterface::init(std::string proxyName)
{
   mProxyName = proxyName;    // Set the name
   mNameSet = true;
   refresh();                 // Refresh the name
}


//! NOTE: If the interface does not have an initialized mProxyName, then don't try to refresh it
void vjDeviceInterface::refresh()
{
   int prev_proxy_index = mProxyIndex;    // Keep track of previous value

   // If it is not initialized, then don't try
   if(!mNameSet)
   { return; }

   mProxyIndex = vjKernel::instance()->getInputManager()->getProxyIndex(mProxyName);
   if (mProxyIndex == -1)
   {
      vjDEBUG(vjDBG_ALL,vjDBG_CONFIG_LVL)
         << "WARNING: vjDeviceInterface::refresh: could not find proxy: "
         << mProxyName.c_str() << std::endl << vjDEBUG_FLUSH;
      vjDEBUG(vjDBG_ALL,vjDBG_CONFIG_LVL)
         << "         Make sure the proxy exists in the current configuration."
         << std::endl << vjDEBUG_FLUSH;
      vjDEBUG(vjDBG_ALL,vjDBG_CONFIG_LVL)
         << "   referencing device interface will be stupified to point at dummy device."
         << std::endl << vjDEBUG_FLUSH;
   }
   else if((mProxyIndex != -1) && (prev_proxy_index == -1))   // ASSERT: We have just gotten a valid proxy to point to
   {
      vjDEBUG(vjDBG_ALL,vjDBG_WARNING_LVL)
         << "vjDeviceInterface::refresh: Success: Now able to find proxy: "
         << mProxyName.c_str() << std::endl << vjDEBUG_FLUSH;
   }
}


void vjDeviceInterface::addDevInterface(vjDeviceInterface* dev)
{ mAllocatedDevices.push_back(dev); }

void vjDeviceInterface::removeDevInterface(vjDeviceInterface* dev)
{ 
   // Attempt to find the device, if found, erase it, if not, then assert
   std::vector<vjDeviceInterface*>::iterator found_dev 
         = std::find(mAllocatedDevices.begin(), mAllocatedDevices.end(), dev);
   vjASSERT(found_dev != mAllocatedDevices.end() && "Tried to remove non-registered interface");
         
   if(mAllocatedDevices.end() != found_dev)
   {
      mAllocatedDevices.erase(found_dev);
   }
}

void vjDeviceInterface::refreshAllDevices()
{
   for(unsigned int i=0;i<mAllocatedDevices.size();i++)
   {
      vjDeviceInterface* dev = mAllocatedDevices[i];
      dev->refresh();
   }
}

std::vector<vjDeviceInterface*> vjDeviceInterface::mAllocatedDevices = std::vector<vjDeviceInterface*>();
