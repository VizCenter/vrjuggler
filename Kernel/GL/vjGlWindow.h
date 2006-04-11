/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998, 1999, 2000 by Iowa State University
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


#ifndef _VJ_GL_WINDOW_H_
#define _VJ_GL_WINDOW_H_

#include <vjConfig.h>
#include <stdio.h>
#include <GL/gl.h>

#include <Kernel/vjDisplay.h>

//-------------------------------------------------------
//: Represent cross-platform interface to OpenGL windows
//-------------------------------------------------------
// This interface is used by the OpenGL draw manager
// in order to keep all platform specific code in this
// one class.
//-------------------------------------------------------
class vjGlWindow
{
public:
   vjGlWindow()
   {
      mWindowId = getNextWindowId();
      in_stereo = false;
      window_is_open = false;
      mDirtyContext = true;      // Always dirty when window first created

      mDisplay = NULL;
   }

   // Cirtual destructor
   virtual ~vjGlWindow()
   {;}

public:

   //: Open the OpenGL window
   //! PRE: this has been configured
   virtual int open(){ return 1;}

   //: Close the OpenGL window
   virtual int close(){return 1;}

   //: Sets the current OpenGL context to this window
   //! POST: this.context is active context
   virtual bool makeCurrent(){return false;}

   //: Configure the window
   //! POST: this' is configured based on the data in display
   virtual void config(vjDisplay* _display);

   //: Performs an OpenGL swap buffers command
   //! POST: a glFlush must be called explicitly by the implementation
   //+       or explicitly by the functions used in the implementation
   virtual void swapBuffers(){;}

   //: Handle any window events that have occured
   //
   virtual void checkEvents() {;}

public:
   //: Sets the projection matrix for this window to draw the left eye frame
   // If the window is in stereo, it changes to the left buffer
   void setLeftEyeProjection();

   //: Sets the projection matrix for this window to draw the right eye frame
   // If the window is in stereo, it changes to the right buffer
   void setRightEyeProjection();

   //: Sets the projection matrix for this window to the one for simulator
   void setCameraProjection();

   //: Is the context dirty?
   bool hasDirtyContext() const
   { return mDirtyContext; }

   //: Set the dirty bit for the context
   void setDirtyContext(bool val=true)
   { mDirtyContext = val; }

   //: Query wether the window is open
   //! RETURNS: true - If window is open
   bool isOpen() const
   { return window_is_open; }

   //: Query wether the window is in stereo
   //! RETURNS: true - If window is in stereo
   bool isStereo()
   { return in_stereo;}

   vjDisplay* getDisplay()
   { return mDisplay;}

   //!RETURNS: A unique window id
   int getId()
   { return mWindowId; }

   friend std::ostream& operator<<(std::ostream& out, vjGlWindow* win);

public:  /**** Static Helpers *****/
   /* static */ virtual bool createHardwareSwapGroup(std::vector<vjGlWindow*> wins)
   {
      vjDEBUG(vjDBG_ALL,0) << "WARNING: hardware swap not supported.\n" << vjDEBUG_FLUSH;
      return false;
   }

protected:
     // we store a pointer to the display that we're
     // created from, to config & to get the viewing
     // transforms from.
   vjDisplay* mDisplay;

   bool mDirtyContext;  //: The context is dirty.  We need to (re)initialize it next draw

   bool in_stereo;      //: Wether the display is actually in stereo
                        // if we wanted a stereo display but couldn't open it
                        // we fall back to mono, and this will be false.
   bool  border;        //: Do we have a border
   bool window_is_open; //: Is the window open
   int  window_width, window_height;
   int  origin_x, origin_y;          //: lower-left corner of window
   int  mWindowId;                  //: A unique window id to identify us

private:
   static int mCurMaxWinId;             // The current maximum window id

   static int getNextWindowId()
   {
      return mCurMaxWinId++;
   }
};

// ostream& operator<<(ostream& out, vjGlWindow& win);

#endif
