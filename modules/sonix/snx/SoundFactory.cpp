
/****************** <SNX heading BEGIN do not edit this line> *****************
 *
 * sonix
 *
 * Original Authors:
 *   Kevin Meinert
 *
 * -----------------------------------------------------------------
 * File:          $RCSfile$
 * Date modified: $Date$
 * Version:       $Revision$
 * -----------------------------------------------------------------
 *
 ****************** <SNX heading END do not edit this line> ******************/
/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998, 1999, 2000, 2001 by Iowa State University
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
 *************** <auto-copyright.pl END do not edit this line> ***************/

//#ifndef __APPLE__

#include <string>
#include <iostream>
#include "snx/ISoundImplementation.h"
#include "snx/StubSoundImplementation.h" // in case lookup fails...

#include "snx/SoundFactory.h" // my header.


namespace snx
{

/**
 * @input name of api to create
 * @output an implementation is returned for you to use
 * @postconditions if apiName is not known, then a stub implementation is returned
 * @semantics factory function used to create an implementation of a sound API 
 */
void SoundFactory::createImplementation( const std::string& apiName,
                           snx::ISoundImplementation* &mImplementation )
{
   if (mRegisteredImplementations.count( apiName ) > 0)
   {
      mRegisteredImplementations[apiName]->clone( mImplementation );
   }

   else
   {
      mImplementation = new snx::StubSoundImplementation;
   }
   
   //mImplementation->setName( apiName );
}

}; // end namespace

//#endif //stub out on OS X
