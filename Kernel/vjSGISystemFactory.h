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


#ifndef _VJ_SGI_SYSTEM_FACTORY_H_
#define _VJ_SGI_SYSTEM_FACTORY_H_

#include <vjConfig.h>
#include <Kernel/vjSystemFactory.h>
#include <Utils/vjSingleton.h>

class vjDisplayManager;

//---------------------------------------------------
//: Concrete class for SGI System specific Factory.
//
//	Concrete class to create SGI system specific objects.
//
// @author Allen Bierbaum
//  Date: 9-9-97
//----------------------------------------------------
class vjSGISystemFactory : public vjSystemFactory
{
public:

   /**
	 * POST: Returns an SGI OpenGL window
    */
    virtual vjGlWindow* getGLWindow();


      // --- Singleton stuff --- //
protected:
   /// Constructor:  Hidden, so no instantiation is allowed
   vjSGISystemFactory()   // Do nothing, but hide
   {;}

   vjSingletonHeader(vjSGISystemFactory);
/*
public:
   /// Get instance of singleton object
   static vjSGISystemFactory* instance()
   {
      if (_instance == NULL)
         _instance = new vjSGISystemFactory;
      return _instance;
   }

private:
   /// The instance
   static vjSGISystemFactory* _instance;
   */

};

#endif
