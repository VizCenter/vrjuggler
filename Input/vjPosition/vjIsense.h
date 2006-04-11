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


//===============================================================
// vjIsense (a Wrapper for aFlock)
//
// Purpose:
//      VR Juggler Ascention Flock of birds tracking class
//
// Author:
// Kevin Meinert
//
// Last Modified: 4-22-99
//===============================================================
#ifndef vjIsense_h
#define vjIsense_h

#include <vjConfig.h>
#include <Input/vjPosition/vjPosition.h>
#include <Input/vjInput/vjDigital.h>
#include <Input/vjInput/vjAnalog.h>
#include <Threads/vjThread.h>
#include <Input/vjPosition/isIntersense.h>
#include <vector>

// maximum number of digital and analog buttons possible on a IS interface
// box.
#ifndef MAX_NUM_BUTTONS
#define MAX_NUM_BUTTONS 8
#endif

#ifndef MAX_NUM_STATIONS
#define MAX_NUM_STATIONS 8
#endif

#ifndef MAX_ANALOG_CHANNELS
#define MAX_ANALOG_CHANNELS 10
#endif

#define IS_BUTTON_NUM MAX_NUM_BUTTONS*MAX_NUM_STATIONS
#define IS_ANALOG_NUM MAX_ANALOG_CHANNELS*MAX_NUM_STATIONS

struct vjIsense_Data {
    int digital[IS_BUTTON_NUM];
    float analog[IS_ANALOG_NUM];
};

typedef struct {
    int stationIndex;
    bool enabled;
    
    int dig_min, dig_num;
    int ana_min, ana_num;
    
    bool useDigital, useAnalog;
} vjISStationConfig;


enum {
    vjIsense_Units_Inches = 0, 
    vjIsense_Units_Feet, 
    vjIsense_Units_Centimeters,
    vjIsense_Units_Meters, 
    vjIsense_Units_COUNT
};

// XXX: It should be virtual public, but that causes an assertion failure.  This needs to be debugged
//class vjIsense : virtual public vjPosition, virtual public vjDigital, virtual public vjAnalog

//----------------------------------------------------------------------------
//: Position derived class for running an IS900 tracking system.
//
//  Wraps the isIntersense driver.
//
//  vjIsense is a positional device driver for the Flock of Birds, the config
//  chunk in the constructor should set up all the settings, for these to be
//  changed the Flock has to be deleted and a new instance created with an
//  updated configchunk.
//  <br>
//! NOTE: vjIsense inherits from vjDigital and vjAnalog.  These base classes,
//+ however, can not handle multiple receivers in the same way as
//+ vjPosition.  Therefore, to access buttons 0-3 on station 0,
//+ call for button 0-3. but to access buttons 0-3 on station 1,
//+ you must ask for units 4-7, and so on.
//! NOTE: Some functions still remain for changing the options of
//+    the flock when its not in Sampling mode, but in order to stay
//+    consistent with the vjInput/vjPosition functionality these
//+    are only left for building apps without vjConfigChunks
//! NOTE: A note on receiver access:
//+  Clients of juggler should access tracker recievers as [0-n]
//+  For example, if you have recievers 1,2, and 4 with transmitter on 3,
//+  then you can access the data, in order, as 0,1,2.
//
// See also: vjPosition
//---------------------------------------------------------------------------
//!PUBLIC_API:
class vjIsense : public vjPosition, public vjDigital,  public vjAnalog
{
    public:
   vjIsense();
   virtual ~vjIsense();


//: configure the flock with a config chunk
    virtual bool config(vjConfigChunk* c);

//: begin sampling
    int startSampling();

// Main thread of control for this active object
    void controlLoop(void* nullParam);

//: stop sampling
    int stopSampling();

//: sample data
    int sample();

//: update to the sampled data.
    void updateData();

//: get the device name
    char* getDeviceName() { return "vjIsense"; }

//: return what chunk type is associated with this class.
    static std::string getChunkType() { return std::string("Isense");}

//: Get the receiver transform
//! ARGS: dev - is the reciever number
//! POST: returns a pointer to the receiver's matrix
//! NOTE: Clients of juggler should access tracker recievers as [0-n]
//+  For example, if you have recievers 1,2, and 4 with transmitter on 3,
//+  then you can access them, in order, as 0,1,2.
    vjMatrix* getPosData( int dev = 0); // 0 base

//: Get the digital and analog data
//! ARGS: d - the button number
//! POST: returns a boolean value where 0 = false and 1 = true
//! NOTE: Since the tracker has multiple possible devices but digital
//+   devices assume only one device, the buttons are layed out as
//+   d = MAX_NUM_BUTTONS*(device number) + (button number)
//+   device number is zero based
//+   button number is zero based
//
//+     ex.  for button number 4 of device 1
//+   buttonValue = getDigitalData( 1*MAX_NUM_BUTTONS + 4);
//+   Most configurations have the wand on port 2... so the device number is 1
//+   The rest button layout is described in the Intersense Manual
//+     (this value is set in the juggler config files)
    int getDigitalData(int d = 0);
    float getAnalogData(int d = 0);

//: Get time of last update for this receiver
//! ARGS: dev - is the reciever number
//! POST: returns a pointer to the reciever's timestamp
    vjTimeStamp* getPosUpdateTime (int dev = 0);

//: see if the flock is active or not
    inline bool isActive() { return mTracker.isActive(); };

private:

    int getStationIndex(int stationNum, int bufferIndex);

    isIntersense mTracker;

    vjIsense_Data mInput[3];
    
    vjISStationConfig* stations;
    
    std::vector<int> mDigitalData;
    std::vector<int> mAnalogData;
    
    float curConvFactor;
    
//KLUDGE: work around the inherent difference between vjPosition and vjDigital (and vjAnalog)
// Motivation: vjPositional expects multiple positional devices to be connected to the same
// port and provides a means for accesses each positional device.  So, if there are four wands
// there is one tracker unit that allows access to each wand...  For digital devices, there
// may be limitless input, but each digital button is attached to the same digital device
// The tracker with digital io uses its natural subset of positional devices to encapsulate
// the digital IO as well. Therefore what is needed with four wands is a digital device that allows
// access to subsets of digital buttons.

};

#endif //vjIsense_h
