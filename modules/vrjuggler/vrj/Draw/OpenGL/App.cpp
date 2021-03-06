/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2010 by Iowa State University
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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 *************** <auto-copyright.pl END do not edit this line> ***************/

#include <vrj/Draw/OpenGL/Config.h>

#include <vrj/Draw/OpenGL/DrawManager.h>
#include <vrj/Draw/OpenGL/App.h>


namespace vrj
{

namespace opengl
{

// NOTE: These member functions are implemented here rather than being
// inlined within vrj/Draw/OpenGL/App.h so that vrj::opengl::App is properly
// compiled into the VR Juggler OpenGL Draw Manager library. Sometimes, full
// inlining is a bad thing.

App::App(Kernel* kern)
   : vrj::App(kern)
{
   /* Do nothing. */ ;
}

App::~App()
{
   /* Do nothing. */ ;
}

vrj::DrawManager* App::getDrawManager()
{
   return vrj::opengl::DrawManager::instance();
}

}

}
