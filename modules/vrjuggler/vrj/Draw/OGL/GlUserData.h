/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2007 by Iowa State University
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

#ifndef _VRJ_GL_USER_DATA_H_
#define _VRJ_GL_USER_DATA_H_

#include <vrj/Draw/OGL/Config.h>

#include <vrj/Kernel/UserPtr.h>
#include <vrj/Display/ViewportPtr.h>
#include <vrj/Draw/OGL/GlWindowPtr.h>


namespace vrj
{

// Declare the class that we need pointers too below
class Projection;

/** \class GlUserData GlUserData.h vrj/Draw/OGL/GlUserData.h
 *
 * Holds data about OpenGL users for draw process.
 *
 * This class holds interesting information that can be used in an OpenGL draw
 * callback to find information about the user.
 *
 * @note These values are only intended for use by advanced users/applications.
 *       By using these values it is possible to create applications that are
 *       non-portable.
 */
class VJ_OGL_CLASS_API GlUserData
{
public:
   GlUserData();

   UserPtr getUser()
   {
      return mUser;
   }

   void setUser(UserPtr user)
   {
      mUser = user;
   }

   Projection* getProjection()
   {
      return mProj;
   }

   void setProjection(Projection* proj)
   {
      mProj = proj;
   }

   ViewportPtr getViewport()
   {
      return mViewport;
   }

   void setViewport(ViewportPtr vp)
   {
      mViewport = vp;
   }

   GlWindowPtr getGlWindow()
   {
      return mGlWindow;
   }

   void setGlWindow(GlWindowPtr win)
   {
      mGlWindow = win;
   }

protected:
   UserPtr      mUser;        /**< The current user we are rendering */
   Projection*  mProj;        /**< The current projection being used */
   ViewportPtr  mViewport;    /**< The current vrj viewport being used */
   GlWindowPtr  mGlWindow;    /**< The current GL window that we are rendering in. (basically the gl context) */
};

}

#endif
