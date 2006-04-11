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


/////////////////////////////////////////////////////////////////////////
//
// positional tracker base class
//
////////////////////////////////////////////////////////////////////////
#ifndef _VJ_ANALOG_H_
#define _VJ_ANALOG_H_

#include <vjConfig.h>
#include <Input/vjInput/vjInput.h>

//-----------------------------------------------------------------------------
//: vjAnalog is the abstract base class that devices with digital data derive
//+ from.
//
//  vjAnalog is the base class that digital devices must derive from.
//  vjAnalog inherits from vjInput, so it has pure virtual function
//  constraints from vjInput in the following functions: StartSampling,
//  StopSampling, Sample, and UpdateData. <br> <br>
//
//  vjAnalog adds one new pure virtual function, GetAnalogData for retreiving
//  the digital data, similar to the addition for vjPosition and vjDigital.
//!PUBLIC_API:
//-----------------------------------------------------------------------------
class vjAnalog : virtual public vjInput
{
public:

	//: Constructor
   //! POST: Set device abilities
   //! NOTE: Must be called from all derived classes
   vjAnalog() : mMin( 0.0f ), mMax( 1.0f ) 
   {
      deviceAbilities = deviceAbilities | DEVICE_ANALOG;
   }
   ~vjAnalog() {}
	
   // Just call base class config
   //! NOTE: Let constructor set device abilities
   virtual bool config(vjConfigChunk* c)
   { 
      mMin = static_cast<float>( c->getProperty("min") );
      float max = static_cast<float>( c->getProperty("max") );
      if (max > 0.0f)
         mMax = max;
      return vjInput::config( c ); 
   }

	/* vjInput pure virtual functions */
	virtual int startSampling() = 0;
	virtual int stopSampling() = 0;
	virtual int sample() = 0;
	virtual void updateData() = 0;
	
	//: Get the device name
	char* getDeviceName() { return "vjAnalog"; }
	
   /* new pure virtual functions */
   //: Return "analog data".. 
   //  Gee, that's ambiguous especially on a discrete system such as a digital computer....
   //  
   //! PRE: give the device number you wish to access.
   //! POST: returns a value that ranges from 0.0f to 1.0f
   //! NOTE: for example, if you are sampling a potentiometer, and it returns reading from 
   //        0, 255 - this function will normalize those values (using vjAnalog::normalizeMinToMax())
   //        for another example, if your potentiometer's turn radius is limited mechanically to return
   //        say, the values 176 to 200 (yes this is really low res), this function will still return
   //        0.0f to 1.0f. 
   //! NOTE: to specify these min/max values, you must set in your vjAnalog (or analog device) config
   //        file the field "min" and "max".  By default (if these values do not appear), 
   //        "min" and "max" are set to 0.0f and 1.0f respectivly.
   //! NOTE: TO ALL ANALOG DEVICE DRIVER WRITERS, you *must* normalize your data using 
   //        vjAnalog::normalizeMinToMax()
	virtual float getAnalogData(int devNum = 0) = 0;
   
protected:
   // give a value that will range from [min() <= n <= max()]
   // return a value that is normalized to the range of 0.0f <= n <= 1.0f
   // if n < 0 or n > 1, then result = 0 or 1 respectively.
   void normalizeMinToMax( const float& plainJaneValue, float& normedFromMinToMax )
   {
      float value = plainJaneValue;

      // first clamp the value so that min<=value<=max
      if (value < mMin) value = mMin;
      if (value > mMax) value = mMax;

      // slide everything to 0.0 (subtract all by mMin)
      float //tmin( 0.0f ), 
            tmax( mMax - mMin), 
            tvalue = value - mMin;

      // since [tmin/tmax...tmax/tmax] == [0.0f...1.0f], the normalized value will be value/tmax
      normedFromMinToMax = tvalue / tmax;
   }


   float getMin() const { return mMin; }
   float getMax() const { return mMax; }
   void setMin( float min ) { mMin = min; }
   void setMax( float max ) { mMax = max; }

   float mMin, mMax;
};


#endif	/* _VJ_ANALOG_H_ */
