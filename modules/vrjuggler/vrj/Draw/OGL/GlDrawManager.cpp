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

#include <vrj/vrjConfig.h>

#include <stdlib.h>
#include <boost/concept_check.hpp>

#include <cluster/ClusterManager.h>

#include <vrj/Display/DisplayManager.h>
#include <vrj/Kernel/Kernel.h>

#include <vrj/Display/Display.h>
#include <vrj/Display/Viewport.h>
#include <vrj/Display/SimViewport.h>
#include <vrj/Display/SurfaceViewport.h>

#include <vrj/Draw/OGL/GlApp.h>

#include <vrj/Draw/OGL/GlPipe.h>
#include <vrj/Draw/OGL/GlWindow.h>
#include <vrj/Draw/OGL/GlSimInterfaceFactory.h>

#include <gmtl/Vec.h>
#include <gmtl/Output.h>

//#include <gadget/Type/Glove.h>
//#include <gadget/Type/GloveProxy.h>

#include <vrj/Draw/OGL/GlDrawManager.h>


namespace vrj
{

vprSingletonImp(GlDrawManager);

GlDrawManager::GlDrawManager()
   : mApp(NULL)
   , drawTriggerSema(0)
   , drawDoneSema(0)
//   , mRuntimeConfigSema(0)
   , mRunning(false)
   , mMemberFunctor(NULL)
   , mControlThread(NULL)
{
}

/** Sets the app the draw should interact with. */
void GlDrawManager::setApp(App* _app)
{
   mApp = dynamic_cast<GlApp*>(_app);

   // We have a new app, so the contexts must be re-initialized
   // so... dirty them all.
   dirtyAllWindows();
   vprASSERT(mApp != NULL);
}

/** Returns the app we are rednering. */
GlApp* GlDrawManager::getApp()
{
   return mApp;
}

/**
 * Do initial configuration for the draw manager.
 * Doesn't do anything right now.
 */
/*
void GlDrawManager::configInitial(jccl::Configuration* cfg)
{
    // Setup any config data
}
*/

/** Starts the control loop. */
void GlDrawManager::start()
{
   // --- Setup Multi-Process stuff --- //
   // Create a new thread to handle the control
   mRunning = true;

   mMemberFunctor =
      new vpr::ThreadMemberFunctor<GlDrawManager>(this, &GlDrawManager::main,
                                                  NULL);
   mControlThread = new vpr::Thread(mMemberFunctor);

   vprDEBUG(vrjDBG_DRAW_MGR, vprDBG_CONFIG_LVL)
      << "vrj::GlDrawManager started (thread: " << mControlThread << ")\n"
      << vprDEBUG_FLUSH;
}


    // Enable a frame to be drawn
    // Trigger draw
void GlDrawManager::draw()
{
   drawTriggerSema.release();
}


/**
 * Blocks until the end of the frame.
 * @post The frame has been drawn.
 */
void GlDrawManager::sync()
{
   drawDoneSema.acquire();
}


/** This is the control loop for the manager. */
void GlDrawManager::main(void* nullParam)
{
   boost::ignore_unused_variable_warning(nullParam);

   while(mRunning)
   {
      //**// Runtime config will happen here
      // Because the kernel is the only one that can trigger it
      // we will be waiting here at that time

      // Wait for trigger
      drawTriggerSema.acquire();

      // While closing the GlDrawManager this thread will be waiting at
      // drawTriggerSema.acquire(). To properly stop this thread we must
      // allow it to fall through the semaphore and not execute drawAllPipes()
      if (mRunning)
      {
         // THEN --- Do Rendering --- //
         drawAllPipes();
      }

      // -- Done rendering --- //
      drawDoneSema.release();

      // Allow run-time config
      //**//mRuntimeConfigSema.release();
         // This is the time that reconfig can happen
      // configProcessPending();
      //**//mRuntimeConfigSema.acquire();
   }
}

void GlDrawManager::drawAllPipes()
{
   vprDEBUG_BEGIN(vrjDBG_DRAW_MGR,vprDBG_HVERB_LVL)
      << "vjGLDrawManager::drawAllPipes: " << std::endl << std::flush
      << vprDEBUG_FLUSH;
   unsigned int pipeNum;

   // RENDER
      // Start rendering all the pipes
   for(pipeNum=0; pipeNum<pipes.size(); pipeNum++)
      pipes[pipeNum]->triggerRender();

      // Wait for rendering to finish on all the pipes
   for(pipeNum=0; pipeNum<pipes.size(); pipeNum++)
      pipes[pipeNum]->completeRender();

   // Barrier for Cluster
   //vprDEBUG(vprDBG_ALL, vprDBG_STATE_LVL) <<  "BARRIER: Going to sleep for: " << num << std::endl << vprDEBUG_FLUSH;
   cluster::ClusterManager::instance()->createBarrier();
   //vprDEBUG(vprDBG_ALL, vprDBG_STATE_LVL) <<  "BARRIER: IS DONE" << std::endl << vprDEBUG_FLUSH;


   // SWAP
      // Start swapping all the pipes
   for(pipeNum=0; pipeNum<pipes.size(); pipeNum++)
      pipes[pipeNum]->triggerSwap();

      // Wait for swapping to finish on all the pipes
   for(pipeNum=0; pipeNum<pipes.size(); pipeNum++)
      pipes[pipeNum]->completeSwap();


   vprDEBUG_END(vrjDBG_DRAW_MGR,vprDBG_HVERB_LVL)
      << "vjGLDrawManager::drawAllPipes: Done" << std::endl << std::flush
      << vprDEBUG_FLUSH;
}

/**
 * Initializes the drawing API (if not already running).
 * @post Control thread is started.
 */
void GlDrawManager::initAPI()
{
   start();
}


/**
 * Callback when display is added to display manager.
 * @pre Must be in kernel controlling thread.
 * @note This function can only be called by the display manager
 *       functioning on behalf of a thread the holds the kernel
 *       reconfiguration lock.
 *       This guarantees that we are not rendering currently.
 *       We will most likely be waiting for a render trigger.
 */
void GlDrawManager::addDisplay(Display* disp)
{
   vprASSERT(disp != NULL);    // Can't add a null display

   vprDEBUG(vrjDBG_DRAW_MGR, vprDBG_STATE_LVL)
      << "vrj::GlDrawManager:addDisplay: " << disp
      << std::endl << vprDEBUG_FLUSH;


   // -- Finish Simulator setup
   int num_vp(disp->getNumViewports());

   for (int i = 0 ; i < num_vp ; i++)
   {
      Viewport* vp = disp->getViewport(i);

      if (vp->isSimulator())
      {
         jccl::ConfigElementPtr vp_element = vp->getConfigElement();

         SimViewport* sim_vp(NULL);
         sim_vp = dynamic_cast<SimViewport*>(vp);
         vprASSERT(NULL != sim_vp);

         sim_vp->setDrawSimInterface(NULL);

         // Create the simulator stuff
         vprASSERT(1 == vp_element->getNum("simulator_plugin") && "You must supply a simulator plugin.");

         // Create the simulator stuff
         jccl::ConfigElementPtr sim_element =
            vp_element->getProperty<jccl::ConfigElementPtr>("simulator_plugin");

         vprDEBUG(vrjDBG_DRAW_MGR, vprDBG_CONFIG_LVL)
            << "GlDrawManager::addDisplay() creating simulator of type '"
            << sim_element->getID() << "'\n" << vprDEBUG_FLUSH;

         DrawSimInterface* new_sim_i =
            GlSimInterfaceFactory::instance()->createObject(sim_element->getID());

         // XXX: Change this to an error once the new simulator loading code is
         // more robust.  -PH (4/13/2003)
         vprASSERT(NULL != new_sim_i && "Failed to create draw simulator");
         sim_vp->setDrawSimInterface(new_sim_i);
         new_sim_i->initialize(sim_vp);
         new_sim_i->config(sim_element);
      }
   }


   // -- Create a window for new display
   // -- Store the window in the wins vector
   // Create the gl window object.  NOTE: The glPipe actually "creates" the opengl window and context later
   GlWindow* new_win = getGLWindow();
   new_win->configWindow(disp);                                            // Configure it
   mWins.push_back(new_win);                                         // Add to our local window list

   // -- Create any needed Pipes & Start them
   unsigned int pipe_num = new_win->getDisplay()->getPipe();    // Find pipe to add it too

   if(pipes.size() < (pipe_num+1))           // ASSERT: Max index of pipes is < our pipe
   {                                         // +1 because if pipeNum = 0, I still need size() == 1
      while(pipes.size() < (pipe_num+1))     // While we need more pipes
      {
         GlPipe* new_pipe = new GlPipe(pipes.size(), this);  // Create a new pipe to use
         pipes.push_back(new_pipe);                          // Add the pipe
         new_pipe->start();                                  // Start the pipe running
                                                             // NOTE: Run pipe even if no windows.  Then it waits for windows.
      }
   }

   // -- Add window to the correct pipe
   GlPipe* pipe;                           // The pipe to assign it to
   pipe = pipes[pipe_num];                    // ASSERT: pipeNum is in the valid range
   pipe->addWindow(new_win);              // Window has been added

   vprASSERT(isValidWindow(new_win));      // Make sure it was added to draw manager
}


/**
 * Callback when display is removed to display manager.
 * @pre disp must be a valid display that we have.
 * @post window for disp is removed from the draw manager and child pipes.
 */
void GlDrawManager::removeDisplay(Display* disp)
{
   GlPipe* pipe;  pipe = NULL;
   GlWindow* win; win = NULL;     // Window to remove

   for(unsigned int i=0;i<mWins.size();i++)
   {
      if(mWins[i]->getDisplay() == disp)      // FOUND it
      {
         win = mWins[i];
         pipe = pipes[win->getDisplay()->getPipe()];
      }
   }

   // Remove the window from the pipe and our local list
   if(win != NULL)
   {
      vprASSERT(pipe != NULL);
      vprASSERT(isValidWindow(win));
      pipe->removeWindow(win);                                                   // Remove from pipe
      mWins.erase(std::remove(mWins.begin(),mWins.end(),win), mWins.end());      // Remove from draw manager
      vprASSERT(!isValidWindow(win));
   }
   else
   {
      vprDEBUG(vprDBG_ERROR, vprDBG_CRITICAL_LVL) << clrOutNORM(clrRED,"ERROR:") << "vrj::GlDrawManager::removeDisplay: Attempted to remove a display that was not found.\n" << vprDEBUG_FLUSH;
      vprASSERT(false);
   }

}


/** Shutdown the drawing API */
void GlDrawManager::closeAPI()
{
   vprDEBUG(vrjDBG_DRAW_MGR, vprDBG_STATE_LVL) << "vrj::GlDrawManager::closeAPI\n" << vprDEBUG_FLUSH;

   mRunning = false;

   drawTriggerSema.release();
   drawDoneSema.acquire();

   // TODO: Must shutdown and delete all pipes.
   // Stop and delete all pipes

   // TODO: We must fix the closing of EventWindows and GlWindows before we can do this.
   // Close and delete all glWindows
}

/**
 * Adds the element to the draw manager config.
 * @pre configCanHandle(element) == true.
 * @post The elements have reconfigured the system.
 */
bool GlDrawManager::configAdd(jccl::ConfigElementPtr element)
{
   boost::ignore_unused_variable_warning(element);
   return false;
}

/**
 * Removes the element from the current configuration.
 * @pre configCanHandle(element) == true.
 * @return success.
 */
bool GlDrawManager::configRemove(jccl::ConfigElementPtr element)
{
   boost::ignore_unused_variable_warning(element);
   return false;
}

/**
 * Can the handler handle the given element?
 * @return false: We can't handle anything.
 */
bool GlDrawManager::configCanHandle(jccl::ConfigElementPtr element)
{
   boost::ignore_unused_variable_warning(element);
   return false;
}


/**
 * Sets the dirty bits off all the gl windows.
 * Dirty all the window contexts.
 */
void GlDrawManager::dirtyAllWindows()
{
    // Create Pipes & Add all windows to the correct pipe
   for(unsigned int winId=0;winId<mWins.size();winId++)   // For each window we created
   {
      mWins[winId]->setDirtyContext(true);
   }
}


bool GlDrawManager::isValidWindow(GlWindow* win)
{
   bool ret_val = false;
   for(unsigned int i=0;i<mWins.size();i++)
      if(mWins[i] == win)
         ret_val = true;

   return ret_val;
}


/// dumps the object's internal state
void GlDrawManager::outStream(std::ostream& out)
{
    out     << clrSetNORM(clrGREEN)
            << "========== GlDrawManager: " << (void*)this << " ========="
            << clrRESET << std::endl
            << clrOutNORM(clrCYAN,"\tapp: ") << (void*)mApp << std::endl
            << clrOutNORM(clrCYAN,"\tWindow count: ") << mWins.size()
            << std::endl << std::flush;

    for(unsigned int i = 0; i < mWins.size(); i++)
    {
       vprASSERT(mWins[i] != NULL);
       out << clrOutNORM(clrCYAN,"\tGlWindow:\n") << *(mWins[i]) << std::endl;
    }
    out << "=======================================" << std::endl;
}

} // end vrj namespace


#if  defined(VPR_OS_Win32)
#  include <vrj/Draw/OGL/GlWindowWin32.h>
#elif defined(VPR_OS_Darwin) && ! defined(VRJ_USE_X11)
#  include <vrj/Draw/OGL/GlWindowOSX.h>
#else
#  include <vrj/Draw/OGL/GlWindowXWin.h>
#endif

namespace vrj
{

vrj::GlWindow* GlDrawManager::getGLWindow()
{
#if  defined(VPR_OS_Win32)
   return new vrj::GlWindowWin32;
#elif defined(VPR_OS_Darwin) && ! defined(VRJ_USE_X11)
   return new vrj::GlWindowOSX;
#else
   return new vrj::GlWindowXWin;
#endif
}

} // end vrj namespace
