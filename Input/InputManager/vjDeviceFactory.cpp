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


#include <vjConfig.h>
#include <Input/InputManager/vjDeviceFactory.h>

// Sims
#include <Input/vjSim/vjSimAnalog.h>
#include <Input/vjSim/vjSimDigital.h>
#include <Input/vjSim/vjSimPosition.h>
#include <Input/vjSim/vjSimGloveGesture.h>
//#include <Input/vjSim/vjSimKeyboardDigital.h>
#include <Input/vjSim/vjSimRelativePosition.h>

// Physical devices
#ifndef WIN32
#include <Input/vjPosition/vjFlock.h>
#include <Input/vjInput/vjIbox.h>
#include <Input/vjGlove/vjCyberGlove.h>
#include <Input/vjKeyboard/vjXWinKeyboard.h>
#include <Input/vjPosition/logiclass.h>
#include <Input/vjPosition/vjMotionStar.h>
#else
#include <Input/vjKeyboard/vjKeyboardWin32.h>
#endif

#include <typeinfo>

// Initialize the singleton ptr
vjDeviceFactory* vjDeviceFactory::mInstance = NULL;

template <class DEV>
vjDeviceConstructor<DEV>::vjDeviceConstructor()
{
   vjASSERT(vjDeviceFactory::instance() != NULL);
   vjDeviceFactory::instance()->registerDevice(this);
}


// Register all the devices that I know about
void vjDeviceFactory::hackLoadKnownDevices()
{
   // NOTE: These will all given unused variable errors in compiling.
   // That is okay, because the don't actually have to do anything.
   // They just register themselves in their constructor.
   vjDeviceConstructor<vjSimAnalog>* sim_analog = new vjDeviceConstructor<vjSimAnalog>;
   vjDeviceConstructor<vjSimDigital>* sim_digital = new vjDeviceConstructor<vjSimDigital>;
   vjDeviceConstructor<vjSimPosition>* sim_position = new vjDeviceConstructor<vjSimPosition>;
   vjDeviceConstructor<vjSimGloveGesture>* sim_glove = new vjDeviceConstructor<vjSimGloveGesture>;
   //vjDeviceConstructor<vjSimKeyboardDigital>* sim_keyboard_digital = new vjDeviceConstructor<vjSimKeyboardDigital>;
   vjDeviceConstructor<vjSimRelativePosition>* sim_relative = new vjDeviceConstructor<vjSimRelativePosition>;
#ifndef WIN32
   vjDeviceConstructor<vjFlock>* flock = new vjDeviceConstructor<vjFlock>;
   vjDeviceConstructor<vjMotionStar>* motion_star = new vjDeviceConstructor<vjMotionStar>;
   vjDeviceConstructor<vjIBox>* ibox = new vjDeviceConstructor<vjIBox>;
   vjDeviceConstructor<vjCyberGlove>* cyber_glove = new vjDeviceConstructor<vjCyberGlove>;
   vjDeviceConstructor<vjXWinKeyboard>* xwin_key = new vjDeviceConstructor<vjXWinKeyboard>;
   vjDeviceConstructor<vjThreeDMouse>* threed_mouse = new vjDeviceConstructor<vjThreeDMouse>;
#else
   vjDeviceConstructor<vjKeyboardWin32>* key_win32 = new vjDeviceConstructor<vjKeyboardWin32>;
#endif
}

void vjDeviceFactory::registerDevice(vjDeviceConstructorBase* constructor)
{
   vjASSERT(constructor != NULL);
   mConstructors.push_back(constructor);     // Add the constructor to the list
   vjDEBUG(vjDBG_INPUT_MGR,1) << "vjDeviceFactory::registerDevice: Device registered for: "
              << constructor->getChunkType().c_str()
              << "   :" << (void*)constructor
              << " type:" << typeid(*constructor).name() << endl << vjDEBUG_FLUSH;
}

// Simply query all device constructors registered looking
// for one that knows how to load the device
bool vjDeviceFactory::recognizeDevice(vjConfigChunk* chunk)
{
   if(findConstructor(chunk) == -1)
      return false;
   else
      return true;
}

//: Load the specified device
//!PRE: recognizeDevice(chunk) == true
//!ARGS: chunk - specification of the device to load
//!RETURNS: null - Device failed to load
//+         other - Pointer to the loaded device
vjInput* vjDeviceFactory::loadDevice(vjConfigChunk* chunk)
{
   vjASSERT(recognizeDevice(chunk));

   int index = findConstructor(chunk);

   vjInput* new_dev;
   vjDeviceConstructorBase* constructor = mConstructors[index];

   vjDEBUG(vjDBG_INPUT_MGR,3) << "vjDeviceFactory::loadDevice: Loading device: "
              << chunk->getType() << "  with: "
              << typeid(*constructor).name() << endl << vjDEBUG_FLUSH;

   new_dev = constructor->createDevice(chunk);
   return new_dev;
}

int vjDeviceFactory::findConstructor(vjConfigChunk* chunk)
{
   std::string chunk_type;
   chunk_type = (std::string)chunk->getType();

   for(unsigned int i=0;i<mConstructors.size();i++)
   {
      // Get next constructor
      vjDeviceConstructorBase* construct = mConstructors[i];
      vjASSERT(construct != NULL);

      if(construct->getChunkType() == chunk_type)
         return i;
   }

   return -1;
}


void vjDeviceFactory::debugDump()
{
   vjDEBUG_BEGIN(vjDBG_INPUT_MGR,0) << "vjDeviceFactory::debugDump\n" << vjDEBUG_FLUSH;
   vjDEBUG(vjDBG_INPUT_MGR,0) << "num constructors:" << mConstructors.size() << "\n" << vjDEBUG_FLUSH;

   for(unsigned int cNum=0;cNum<mConstructors.size();cNum++)
   {
      vjDeviceConstructorBase* dev_constr = mConstructors[cNum];
      vjDEBUG(vjDBG_INPUT_MGR,0) << cNum << ": Constructor:" << (void*)dev_constr
                 << "   type:" << typeid(*dev_constr).name() << "\n" << vjDEBUG_FLUSH;
      vjDEBUG(vjDBG_INPUT_MGR,0) << "   recog:" << dev_constr->getChunkType().c_str() << "\n" << vjDEBUG_FLUSH;
   }

   vjDEBUG_END(vjDBG_INPUT_MGR,0) << "------ END DUMP ------\n" << vjDEBUG_FLUSH;
}
