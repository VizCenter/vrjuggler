/*
 * VRJuggler
 *   Copyright (C) 1997,1998,1999,2000
 *   Iowa State University Research Foundation, Inc.
 *   All Rights Reserved
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
 */


#ifndef _VJ_SIM_DIGITALGLOVE_H
#define _VJ_SIM_DIGITALGLOVE_H
//#pragma once

#include <vjConfig.h>
#include <Input/vjGlove/vjPinchGlove.h>
#include <Input/vjSim/vjSimInput.h>

#include <Input/vjGlove/vjFinger.h>
#include <Input/vjGlove/vjHand.h>

//: Simulated digital device
// Simulates a digital device from a keyboard device.
// It allows any number of simulated digital devices to be created.
//
// When the key configured is held down, the digital device is on.
// When the key is released, the device is no longer set.
//
// This class should not be used directly by the user.
//!PUBLIC_API:
class vjSimDigitalGlove : virtual public vjDigital, virtual public vjSimInput, virtual public vjGlove
{
public:
   //: Default Constructor
   vjSimDigitalGlove();
   
   //: Destructor
   virtual ~vjSimDigitalGlove();

   //: Takes a config chunk
   //  The Juggler API calls this
   virtual bool config( vjConfigChunk* chunk );

   //: Get the digital data for the given "finger"
   //  Returns digital 0 or 1, if "finger" makes sense.<BR>
   //  Returns -1 if function fails or if devNum is out of range.<BR>
   //  NOTE: If devNum is out of range, function will fail, possibly issueing 
   //  an error to a log or console - but will not ASSERT.<BR><BR>
   //  Return Value: 0 == open, 1 == contact.
   //
   //  Use one of these indices to get the glove's digital data<BR>
   //  EX: int result = mGlove.getDigitalData( vjSimDigitalGlove::LTHUMB );
   //  NOTE: These should be the same integers as vjPinchGlove's
   enum finger 
   {
	   LTHUMB = 0, LINDEX = 1, LMIDDLE = 2, LRING = 3, LPINKY = 4, 
	   RTHUMB = 6, RINDEX = 7, RMIDDLE = 8, RRING = 9, RPINKY = 10
   };
   // dev = finger (see finger enum above)
   virtual int getDigitalData( int dev = 0 )
   {
      //vjDEBUG(vjDBG_ALL,0)<<"*** vjSimDigitalGlove::getDigitalData("<<dev<<")\n"<< vjDEBUG_FLUSH;
      vjASSERT( dev < (int)mDigitalData.size() );    // Make sure we have enough space
      return mDigitalData[dev];
   }

   /* These functions don't do anything */
   virtual int startSampling() 
   { 
     //vjDEBUG(vjDBG_ALL,3)<<"start\n"<<vjDEBUG_FLUSH; 
     return 1; 
   }
   virtual int stopSampling() 
   { 
     //vjDEBUG(vjDBG_ALL,3)<<"stop\n"<<vjDEBUG_FLUSH; 
     return 1; 
   }
   virtual int sample() 
   { 
     //vjDEBUG(vjDBG_ALL,3)<<"sample\n"<<vjDEBUG_FLUSH; 
     return 1; 
   }

   //: Update the data
   virtual void updateData();

   void updateFingerAngles();

   //: Get the name of the digital device
   virtual char* getDeviceName() { return "vjSimDigitalGlove";}
   static std::string getChunkType() { return std::string("SimDigitalGlove");}

// vjGesture stuff:
   //: Load trained data for the gesture object
   // Loads the file for trained data
   //void loadTrainedFile(std::string fileName);
   
   //: Get the current gesture.
   //! RETURNS: id of current gesture
   //virtual int getGesture();
   
private:
   std::vector<int>          mDigitalData;   //: The digital data that we have
   std::vector<vjKeyModPair> mSimKeys;       //: The keys to press for digital simulation

   vjHand                     mLeftHand, mRightHand;   
   
   //int                     mCurGesture;   //: The current gesture id
};

#endif
