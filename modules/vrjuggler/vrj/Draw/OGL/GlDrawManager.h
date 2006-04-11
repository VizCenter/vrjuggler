/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2003 by Iowa State University
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

#ifndef _VRJ_GL_DRAW_MANAGER_H_
#define _VRJ_GL_DRAW_MANAGER_H_

#include <vrj/vrjConfig.h>
#include <vector>

#include <vpr/vpr.h>
//#include <vpr/Sync/CondVar.h>
#include <vpr/Sync/Semaphore.h>
#include <vpr/Util/Singleton.h>
#include <vpr/Thread/Thread.h>
#include <vpr/Thread/TSObjectProxy.h>

#ifdef VPR_OS_Darwin
#   include <OpenGL/gl.h>
#   include <OpenGL/glu.h>
#else
#   include <GL/gl.h>
#   include <GL/glu.h>
#endif

#include <jccl/Config/ConfigElementPtr.h>

#include <vrj/Draw/DrawManager.h>
//#include <vrj/Draw/OGL/GlApp.h>
#include <vrj/Draw/OGL/GlWindow.h>
#include <vrj/Draw/OGL/GlPipe.h>
#include <vrj/Draw/OGL/GlUserData.h>

namespace vrj
{
   class GlApp;
   class SimViewport;

/**
 * Concrete Singleton Class for OpenGL drawing.
 *
 * Responsible for all OGL based rendering.
 *
 * GlDrawManager is an active object.  It manages
 * ogl pipes and windows.  In addition, it triggers
 * rendering, swapping, and syncing of the windows
 * under it's control.
 *
 * All access to the ogl rendering structures has
 * to happen from the control thread or in the case
 * of context sensitive functions, from the control
 * thread of the managed pipes. Because of this,
 * the object uses queues to hold new windows.
 *
 * @date 1-7-98
 */
class VJ_CLASS_API GlDrawManager : public DrawManager
{
public:
   friend class GlPipe;
   friend class GlContextDataBase;

   /**
    * Function to config API specific stuff.
    * Takes a jccl::Configuration and extracts API-specific stuff.
    */
   //**//virtual void configInitial(jccl::Configuration*  cfg);

   /** Starts the control loop. */
   virtual void start();

   /** Enables a frame to be drawn. */
   virtual void draw();

   /**
    * Blocks until the end of the frame.
    *
    * @post The frame has been drawn.
    */
   virtual void sync();

   /** Control loop for the manager. */
   void main(void* nullParam);

   /** Initializes the drawing API (if not already running). */
   virtual void initAPI();

   /**
    * Callback when display is added to display manager.
    *
    * @note This function can only be called by the display manager
    *       functioning in the kernel thread to signal a new display added
    *       This guarantees that we are not rendering currently.
    *       We will most likely be waiting for a render trigger.
    */
   virtual void addDisplay(Display* disp);

   /** Callback when display is removed to display manager. */
   virtual void removeDisplay(Display* disp);

   /** Shutdown the drawing API. */
   virtual void closeAPI();

   /** Outputs some debug info. */
   virtual void outStream(std::ostream& out);

   /** Draws all the ogl pipes/windows. */
   void drawAllPipes();

   /** Set the app the draw should interact with. */
   virtual void setApp(App* _app);

   /** Returns the app we are rendering. */
   GlApp* getApp();

   //void setDisplayManager(DisplayManager* _dispMgr);

public: // Config element handlers
   /**
    * Adds the element to the configuration.
    *
    * @pre configCanHandle(element) == true
    * @return success
    */
   virtual bool configAdd(jccl::ConfigElementPtr element);

   /**
    * Removes the element from the current configuration.
    *
    * @pre configCanHandle(element) == true
    * @return success
    */
   virtual bool configRemove(jccl::ConfigElementPtr element);

   /**
    * Can the handler handle the given config element?
    *
    * @return true if we can handle it; false if we can't.
    */
   virtual bool configCanHandle(jccl::ConfigElementPtr element);


public:
   /**
    * Gets ptr to the current user data.  Should be used in the draw function.
    *
    * @note This user data is valid ONLY in draw().  It is not valid anywhere
    *       else.
    */
   GlUserData* currentUserData();

   /**
    * Returns a unique identifier for the current context.
    *
    * @note This id is ONLY valid in contextInit() and draw().
    */
   int getCurrentContext();

protected:
   /**
    * Factory function to get system specific OpenGL window.
    *
    * @post Returns an OpenGL window for the current system.
    */
   GlWindow* getGLWindow();

   void setCurrentContext(int val);

   /** Sets the dirty bits off all the GL windows. */
   void dirtyAllWindows();

   /** Is the window a valid window for the draw manager? */
   bool isValidWindow(GlWindow* win);


protected:
   // --- Config Data --- //
   int      numPipes;     /**<  The number of pipes in the system */

   // --- API data --- //
   GlApp*                 mApp;      /**< The OpenGL application */
   std::vector<GlWindow*> mWins;     /**< A list of the windows in the system */
   std::vector<GlPipe*>   pipes;    /**< A list of the pipes in the system */

   // --- Helper field data --- //
   vpr::TSObjectProxy<int>           mContextId;   /**<  TS Data for context id */
   vpr::TSObjectProxy<GlUserData>    mUserData;    /**  User data for draw func */

   // --- MP Stuff -- //
   vpr::Semaphore    drawTriggerSema;  /**< Semaphore for draw trigger */
   vpr::Semaphore    drawDoneSema;     /**< Semaphore for drawing done */
   //vpr::Semaphore    mRuntimeConfigSema;  /**< Protects run-time config.  Only when this semaphore
   //                                     *  is acquired can run-time config occur */
   bool              mRunning;         /**< Used to stop the drawing thread. */

   vpr::ThreadMemberFunctor<GlDrawManager>* mMemberFunctor;
   vpr::Thread* mControlThread;

protected:
   GlDrawManager();

   virtual ~GlDrawManager()
   {
      // XXX: Need to shut down the control thread and free the memory pointed
      // to by mMemberFunctor and mControlThread.
   }

   GlDrawManager(const GlDrawManager& o)
      : DrawManager(o)
   {;}

   void operator=(const GlDrawManager&) {;}

   vprSingletonHeader(GlDrawManager);
};

} // end namespace

#endif
