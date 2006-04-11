/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998, 1999, 2000 by Iowa State University
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


#ifndef _VJ_PINCH_GLOVE_H_
#define _VJ_PINCH_GLOVE_H_

#include <vjConfig.h>
#include <Input/vjGlove/vjGlove.h>
#include <Input/vjInput/vjDigital.h>
#include <Input/vjGlove/fsPinchGlove.h>
#include <string>

#include <Input/vjGlove/vjFinger.h>
#include <Input/vjGlove/vjHand.h>


//: Fakespace Pinchglove Device
//!PUBLIC_API:
class vjPinchGlove : public vjGlove, public vjDigital
{
public:
   //: Construct
   vjPinchGlove() : mGlove( NULL )
   {;}

   //: Destroy the glove
   virtual ~vjPinchGlove();

   virtual bool config( vjConfigChunk* c );

   virtual char* getDeviceName() { return "vjPinchGlove";}
   static std::string getChunkType() { return std::string("PinchGlove");}

   virtual int startSampling();
   virtual int stopSampling();
   virtual int sample();
   virtual void updateData ();

   //: Get the digital data for the given devNum
   //  Returns digital 0 or 1, if devNum makes sense.<BR>
   //  Returns -1 if function fails or if devNum is out of range.<BR>
   //  NOTE: If devNum is out of range, function will fail, possibly issueing 
   //  an error to a log or console - but will not ASSERT.<BR><BR>
   //  0 == open, 1 == contact.
   //
   //  Use one of these indices to get the glove's digital data<BR>
   //  EX: int result = mGlove.getDigitalData( vjPinchGlove::LTHUMB );
   enum vjFinger 
   {
	   LTHUMB = 0, LINDEX = 1, LMIDDLE = 2, LRING = 3, LPINKY = 4, 
	   RTHUMB = 6, RINDEX = 7, RMIDDLE = 8, RRING = 9, RPINKY = 10
   };
   virtual int getDigitalData(int devNum = 0);
   
protected:
	//: The main control loop for the object
   void controlLoop( void* nullParam );

   void copyDataFromGlove();
   void updateFingerAngles();

protected:
   //vjThread*         mControlThread;      // The thread of control for the object
   fsPinchGlove*      mGlove;              // The actual glove
   
   vjHand left, right;
};


#endif	/* _VJ_PINCH_GLOVE_H_ */
