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

#ifndef _VJ_SOUND_ENGINE_CONSTRUCTOR_H_
#define _VJ_SOUND_ENGINE_CONSTRUCTOR_H_

#include <Sound/vjSoundEngine.h>
#include <string>
#include <Config/vjConfigChunk.h>
#include <Kernel/vjDebug.h>
#include <Sound/vjSoundFactory.h>


// Apparently this is needed here so that this will compile with GCC.
class vjSoundFactory;

//: Base class for virtual construction of sound engines
// Implementations of this class are registered with the device factory
// for each device in the system
//!PUBLIC_API:
class vjSoundEngineConstructorBase
{
public:
   //: Constructor
   //! POST: Device is registered
   vjSoundEngineConstructorBase() {;}

   //: Create the device
   virtual vjSoundEngine* createEngine(vjConfigChunk* chunk)
   {
      vjDEBUG(vjDBG_ALL,0) << "ERROR: vjSoundEngineConstructorBase::createEngine: Should never be called" << vjDEBUG_FLUSH;
      return NULL;
   }

   //: Get the string desc of the type of chunk we can create
   virtual std::string getChunkType()
   {
      return std::string("BaseConstructor: Invalid type"); 
   }
};


template <class ENGINE>
class vjSoundEngineConstructor : public vjSoundEngineConstructorBase
{
public:
   vjSoundEngineConstructor();

   virtual vjSoundEngine* createEngine(vjConfigChunk* chunk)
   {
      ENGINE* new_engine = new ENGINE;
      bool success = new_engine->config(chunk);
      if(success)
      {
         return new_engine;
      } else {
         delete new_engine;
         return NULL;
      }
   }

   virtual std::string getChunkType()
   { return ENGINE::getChunkType(); }
};


// self registrator with the soundfactory.
template <class ENGINE>
vjSoundEngineConstructor<ENGINE>::vjSoundEngineConstructor()
{
   vjASSERT(vjSoundFactory::instance() != NULL);
   vjSoundFactory::instance()->registerConstructor( this );
}

#endif
