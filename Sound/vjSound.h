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

#ifndef _VJ_SOUND_h
#define _VJ_SOUND_h

#include <iostream>
#include <Math/vjVec3.h>

class vjSoundEngine;

//: vjSound
// a sound
//!PUBLIC_API:
class vjSound
{  
public:
   vjSound( vjSoundEngine& engine )
     : mLooping( 0 ),  mEngine(&engine), mEnabled( true ),
       mPosition( 0.0f, 0.0f, 0.0f ), mVolume( 1.0f ), mFalloff( 20.0f )
   {
   }
   
   virtual ~vjSound()
   {
   }
   
   // loads the sound from a file
   // reutrns false if didn't load.
   virtual bool load( const char* const soundname )
   {
      mName = soundname;
      // be stupid and return true,
      // this allows use of the Sound class to stub out sound.
      // so... of course the sound loaded - this is a stub,
      // it's always a success!
      return true;
   }
   
   const std::string& getName() const { return mName; }
   
   void setEngine( vjSoundEngine& se )
   {
      mEngine = &se;
   }
   
   vjSoundEngine& engine() { return *mEngine; }
   const vjSoundEngine& engine() const { return *mEngine; }
   
   virtual void pitchBend( float amount = 1.0 )
   {
   }
   
   // set to 0 to never loop
   // set to -1 to always loop
   // set to x to loop x number of times.
   virtual void setLooping( int times )
   {
      mLooping = times;
   } 
      
   // allows a user to enable (restart) or disable (pause) a sound 
   // without worring about its state set by start or stop.
   // NOTE: to hear a sound enable() and trigger() must be active.
   //       default is true
   virtual void enable( const bool& state = true ) { mEnabled = state; }
   
   // returns vjSound::ON or vjSound::OFF
   const bool& isEnabled() const { return mEnabled; }
   
   // trigger (play) the sound
   virtual void trigger() {}
   
   // stop the sound.
   // stops the sound before it finishes playing
   virtual void stop() {}
   
   // change the position of the sound.
   virtual void setPosition( float x, float y, float z ) 
   {
      mPosition.set( x, y, z );
   }
   
   void getPosition( float& x, float& y, float& z ) const
   {
      x = mPosition[0];
      y = mPosition[1];
      z = mPosition[2];
   }
   
   // output to stdout, useful for debugging.
   virtual void print() {}
   
   int mLooping;
   
   // calculate the sound volume based on the observer position, sound position, falloff, and volume
   void getPercievedVolume( float& volume ) const;
   
   // set the volume when observer position is same as sound position.
   // vol is between 0.0 and 1.0, where 1.0 is full volume.
   virtual void setVolume( const float& vol )
   {
      mVolume = vol;
   }   
   
   void getVolume( float& vol ) const
   {
      vol = mVolume;
   }
   
   // how fast the volume drops as you move away.
   // set fall == distace at which volume becomes 0
   virtual void setFalloff( const float& vol )
   {
      mVolume = vol;
   }   
   
   void getFalloff( float& vol ) const
   {
      vol = mVolume;
   }   

protected:
   // my associated engine...
   vjSoundEngine* mEngine;
   bool mEnabled;
   std::string mName;
   vjVec3 mPosition; // position of this sound in distance units where 1 is one unit.
   float mVolume;  // from 0.0 to 1.0
   float mFalloff; // in distance units where 1 is one unit.
};

#endif
