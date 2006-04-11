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


#include <vjConfig.h>
#include <Input/vjSim/vjSimDigital.h>


//: Default Constructor
vjSimDigital::vjSimDigital() : vjDigital(), vjSimInput()
{
   vjDEBUG(vjDBG_ALL,4)<<"*** vjSimDigital::vjSimDigital()\n"<< vjDEBUG_FLUSH;
}

//: Destructor
vjSimDigital::~vjSimDigital() 
{
   //vjDEBUG(vjDBG_ALL,4)<<"*** vjSimDigital::~vjSimDigital()\n"<< vjDEBUG_FLUSH;
}

bool vjSimDigital::config(vjConfigChunk* chunk)
{
   //vjDEBUG(vjDBG_ALL,4)<<"*** vjSimDigital::config()\n"<< vjDEBUG_FLUSH;
   
   if((!vjDigital::config(chunk)) || (!vjSimInput::config(chunk)))
      return false;

   std::vector<vjVarValue*> key_list = chunk->getAllProperties("keyPairs");
   mSimKeys = readKeyList(key_list);

   int num_pairs = mSimKeys.size();
   mDigitalData = std::vector<int>(num_pairs,0);      // Initialize to all zeros

   return true;
}

// Updates the state of the digital data vector <br>
//
// NOTE: Digital is on when key is held down
//+     When key is release, digital goes to off state
void vjSimDigital::updateData()
{
   //vjDEBUG(vjDBG_ALL,4)<<"*** vjSimDigital::updateData()\n"<< vjDEBUG_FLUSH;
   
   // -- Update digital data --- //
   for (unsigned int i = 0; i < mSimKeys.size(); i++)
   {
      if(checkKeyPair(mSimKeys[i]))             // If keys pressed
         mDigitalData[i] = 1;
      else
         mDigitalData[i] = 0;
   }
}
