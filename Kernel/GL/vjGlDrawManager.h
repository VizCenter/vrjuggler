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


#ifndef _VJ_GL_DRAW_MANAGER_
#define _VJ_GL_DRAW_MANAGER_

#include <vjConfig.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <Kernel/vjDrawManager.h>
//#include <Kernel/GL/vjGlApp.h>
class vjGlApp;
#include <Kernel/GL/vjGlWindow.h>
#include <Kernel/GL/vjGlPipe.h>
#include <Threads/vjTSObjectProxy.h>
#include <Kernel/GL/vjGlUserData.h>

class vjConfigChunkDB;
class vjSimDisplay;
class vjGloveProxy;

//#include <Sync/vjCond.h>
#include <Sync/vjSemaphore.h>

//-----------------------------------------------
//: Concrete Singleton Class for OpenGL drawing
//
//    Responsible for all OGL based rendering.
//
//  vjGlDrawManager is an active object.  It manages
// ogl pipes and windows.  In addition, it triggers
// rendering, swapping, and syncing of the windows
// under it's control.
//
// All access to the ogl rendering structures has
// to happen from the control thread or in the case
// of context sensitive functions, from the control
// thread of the managed pipes. Because of this,
// the object uses queues to hold new windows.
//
// @author Allen Bierbaum
//  Date: 1-7-98
//------------------------------------------------
//! PUBLIC_API:
class vjGlDrawManager : public vjDrawManager
{
public:
   friend class vjGlPipe;
   friend class vjGlContextDataBase;

    //: Function to config API specific stuff.
    // Takes a chunkDB and extracts API specific stuff
   //**//virtual void configInitial(vjConfigChunkDB*  chunkDB);

   //: Start the control loop
   virtual void start();

   //: Enable a frame to be drawn
   virtual void draw();

    //: Blocks until the end of the frame
    //! POST: The frame has been drawn
   virtual void sync();

   //: Control loop for the manager
   void main(void* nullParam);

   //: Initialize the drawing API (if not already running)
   virtual void initAPI();

   //: Initialize the drawing state for the API based on
   // the data in the display manager.
   //
   //! PRE: API is running (initAPI has been called)
   //! POST: API is ready do draw <br>
   //+    Process model is configured <br>
   //  Multi-pipe data is set <br>
   //  Window list is correct
   virtual void initDrawing();

   //: Callback when display is added to display manager
   //! NOTE: This function can only be called by the display manager
   //+      functioning in the kernel thread to signal a new display added
   //+      This guarantees that we are not rendering currently.
   //+      We will most likely be waiting for a render trigger.
   virtual void addDisplay(vjDisplay* disp);

   //: Callback when display is removed to display manager
   virtual void removeDisplay(vjDisplay* disp);

   //: Shutdown the drawing API
   virtual void closeAPI();

   //: Output some debug info
   virtual void outStream(ostream& out);

   //: Draw all the ogl pipes/windows
   void drawAllPipes();

   //: Set the app the draw should interact with.
   virtual void setApp(vjApp* _app);

   //: Return the app we are rendering
   vjGlApp* getApp();

   void setDisplayManager(vjDisplayManager* _dispMgr);

public: // Chunk handlers
   //: Add the chunk to the configuration
   //! PRE: configCanHandle(chunk) == true
   //! RETURNS: success
   virtual bool configAdd(vjConfigChunk* chunk);

   //: Remove the chunk from the current configuration
   //! PRE: configCanHandle(chunk) == true
   //!RETURNS: success
   virtual bool configRemove(vjConfigChunk* chunk);

   //: Can the handler handle the given chunk?
   //! RETURNS: true - Can handle it
   //+          false - Can't handle it
   virtual bool configCanHandle(vjConfigChunk* chunk);


public:  // Drawing functions used by library
   //: Draw any objects that the manager needs to display
   // i.e. Gloves, etc
   void drawObjects();

   //: Draw projections in Opengl
   void drawProjections(vjSimDisplay* sim);

   //: Draw a simulator using OpenGL commands
   //! NOTE: This is called internally by the library
   void drawSimulator(vjSimDisplay* sim);

protected:     // --- Geom helpers --- //
   void initQuadObj();
   void drawLine(vjVec3& start, vjVec3& end);
   void drawSphere(float radius, int slices, int stacks);
   void drawCone(float base, float height, int slices, int stacks);
   void drawBox(float size, GLenum type);
   void drawWireCube(float size);
   void drawSolidCube(float size);
   void drawGlove(vjGloveProxy* gloveProxy);

   GLUquadricObj* mQuadObj;

public:
   //: Get ptr to the current user data
   // Should be used in the draw function
   //! NOTE: This user data is valid ONLY
   //+ in contextInit() and draw()
   vjGlUserData* currentUserData()
   { return &(*mUserData); }

   //: Returns a unique identifier for the current context
   //! NOTE: This id is ONLY valid in
   //+ contextInit() and draw()
   int getCurrentContext()
   { return (*mContextId); }

protected:
   void setCurrentContext(int val)
   { (*mContextId) = val; }

   //: Set the dirty bits off all the gl windows
   void dirtyAllWindows();

   //: Is the window a valid window for the draw manager
   bool isValidWindow(vjGlWindow* win);


protected:
   // --- Config Data --- //
   int      numPipes;     //: The number of pipes in the system

   // --- API data --- //
   vjGlApp*                 mApp;      //: The OpenGL application
   std::vector<vjGlWindow*> mWins;     //: A list of the windows in the system
   std::vector<vjGlPipe*>   pipes;    //: A list of the pipes in the system

   // --- Helper field data --- //
   vjTSObjectProxy<int>             mContextId;    //: TS Data for context id
   vjTSObjectProxy<vjGlUserData>    mUserData;     //: User data for draw func

   // --- MP Stuff -- //
   vjSemaphore    drawTriggerSema;  // Semaphore for draw trigger
   vjSemaphore    drawDoneSema;     // Semaphore for drawing done
   vjSemaphore    mRuntimeConfigSema;  //: Protects run-time config.  Only when this semaphore
                                       //+ is acquired can run-time config occur


   // --- Singleton Stuff --- //
public:
   static vjGlDrawManager* instance()
   {
      if (_instance == NULL)
         _instance = new vjGlDrawManager();
      return _instance;
   }
protected:
   vjGlDrawManager() : drawTriggerSema(0), drawDoneSema(0), mRuntimeConfigSema(0)
   {
      mQuadObj = NULL;
   }

   virtual ~vjGlDrawManager() {}

private:
   static vjGlDrawManager* _instance;
};

#endif
