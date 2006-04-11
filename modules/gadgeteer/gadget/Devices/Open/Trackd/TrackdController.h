/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2002 by Iowa State University
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

#ifndef _GADGET_TRACKD_CONTROLLER_H_
#define _GADGET_TRACKD_CONTROLLER_H_

#include <gadget/gadgetConfig.h>
#include <vector>
#include <gadget/Type/Input.h>
#include <gadget/Type/Digital.h>
#include <gadget/Type/Analog.h>
#include <gadget/Devices/Sim/SimInput.h>
#include <gadget/Type/InputMixer.h>
#include <gadget/Devices/Open/Trackd/TrackdControllerStandalone.h>

namespace gadget
{


//-----------------------------------------------------------------------------
//: Class interfacing with trackd sensor data
//+ located on the local machine in a shared memory arena
//
//! NOTE: A note on reciever access:
//+  Clients of Juggler should access tracker recievers as [0-n].  For
//+  example, if you have recievers 1,2, and 4 with transmitter on 3, then
//+  you can access the data, in order, as 0, 1, 2.
//
// See also: Digital, Analog
//-----------------------------------------------------------------------------
//!PUBLIC_API:
//class TrackdController : public Input, public Digital, public Analog
class TrackdController : public InputMixer<InputMixer<Input,Digital>,Analog>
{
public:

   // ------------------------------------------------------------------------
   //: Constructor.
   // ------------------------------------------------------------------------
   TrackdController();

   // ------------------------------------------------------------------------
   //: Destructor.
   //
   //! PRE: None.
   //! POST: Shared memory is released
   // ------------------------------------------------------------------------
   ~TrackdController();

   // ------------------------------------------------------------------------
   //: Configure the trackd controller with the given config chunk.
   //
   //! PRE: c must be a chunk that has trackd controller config information
   //! POST: If c is a valid config chunk, the device is configured using its
   //+       contents.  Otherwise, configuration fails and false is returned
   //+       to the caller.
   //
   //! ARGS: c - A pointer to a config chunk.
   //
   //! RETURNS: true - The device was configured succesfully.
   //! RETURNS: false - The config chunk is invalid.
   // ------------------------------------------------------------------------
   virtual bool config(jccl::ConfigChunkPtr c);

   //: Begin sampling
   int startSampling() { return 1;}

   //: Stop sampling
   int stopSampling() { return 1;}

   //: Sample a value
   int sample() { return 1;}

   // ------------------------------------------------------------------------
   //: Update to the sampled data.
   //
   //! PRE: None.
   //! POST: Most recent value is copied over to temp area
   // ------------------------------------------------------------------------
   void updateData();

   //: Return what chunk type is associated with this class.
   static std::string
   getChunkType () {
      return std::string("TrackdController");
   }

   //: Return digital data
   virtual const DigitalData getDigitalData(int devNum=0)
   {
      vprASSERT(devNum < (int)mCurButtons.size() && "Digital index out of range");    // Make sure we have enough space
      return (mCurButtons[devNum]);
   }

   //: Return "analog data"..
   //  Gee, that's ambiguous especially on a discrete system such as a digital computer....
   //
   //! PRE: give the device number you wish to access.
   //! POST: returns a value that ranges from 0.0f to 1.0f
   //! NOTE: for example, if you are sampling a potentiometer, and it returns reading from
   //        0, 255 - this function will normalize those values (using Analog::normalizeMinToMax())
   //        for another example, if your potentiometer's turn radius is limited mechanically to return
   //        say, the values 176 to 200 (yes this is really low res), this function will still return
   //        0.0f to 1.0f.
   //! NOTE: to specify these min/max values, you must set in your Analog (or analog device) config
   //        file the field "min" and "max".  By default (if these values do not appear),
   //        "min" and "max" are set to 0.0f and 1.0f respectivly.
   //! NOTE: TO ALL ANALOG DEVICE DRIVER WRITERS, you *must* normalize your data using
   //        Analog::normalizeMinToMax()
   virtual AnalogData* getAnalogData(int devNum=0)
   {
      vprASSERT(devNum < (int)mCurValuators.size() && "Analog index out of range");    // Make sure we have enough space
      return &(mCurValuators[devNum]);
//        float normalized;
//        this->normalizeMinToMax( mCurValuators[devNum], normalized );
//        return normalized;
   }

private:
   TrackdControllerStandalone* mTrackdController;  // The controller we are dealing with
   std::vector<AnalogData>      mCurValuators;          // The current (up-to-date) values
   std::vector<DigitalData>       mCurButtons;            // The current button states
};

} // End of gadget namespace

#endif

