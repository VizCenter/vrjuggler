/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2002 by Iowa State University
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
#include <string.h>
#include <vrj/vrjParam.h>
#include <vrj/Kernel/Kernel.h>
#include <vrj/Util/Debug.h>
//#include <vrj/Kernel/ConfigManager.h>
#include <vrj/Draw/DrawManager.h>
#include <vrj/Display/DisplayManager.h>
#include <vrj/Kernel/App.h>
#include <vrj/Kernel/User.h>

#include <vpr/vpr.h>
#include <vpr/Thread/Thread.h>
#include <vpr/System.h>
#include <vpr/Util/Version.h>
#include <gadget/Util/Version.h>

#include <gadget/InputManager.h>

#include <jccl/Config/ConfigChunk.h>
#include <jccl/Config/ChunkFactory.h>
//#include <jccl/Net/JackalServer.h>
#include <jccl/RTRC/ConfigManager.h>
#include <jccl/PerfMonitor/PerformanceMonitor.h>

// Get the system factory we need
#if defined(VPR_OS_IRIX) || defined(VPR_OS_Linux) || defined(VPR_OS_AIX) ||   \
    defined(VPR_OS_Solaris) || defined(VPR_OS_FreeBSD) || defined(VPR_OS_HPUX)
#include <vrj/Kernel/SystemFactoryUNIX.h>
#elif defined(VPR_OS_Darwin)
#include <vrj/Kernel/SystemFactoryOSX.h>
#elif defined(VPR_OS_Win32)
#include <vrj/Kernel/SystemFactoryWin32.h>
#endif

namespace vrj
{

//vjKernel* Kernel::_instance = NULL;
vprSingletonImp(Kernel);

/// Starts the Kernel loop running.
int Kernel::start()
{
   if(mControlThread != NULL) // Have already started
   {
      vprDEBUG(vprDBG_ERROR,0) << clrOutNORM(clrRED,"ERROR:") << "vjKernel::start called when kernel already running\n" << vprDEBUG_FLUSH;
      vprASSERT(false);
      exit(0);
   }

   mIsRunning = true;
   initSignalButtons();    // Initialize the signal buttons that may be pressed

   // Create a new thread to handle the control
   vpr::ThreadMemberFunctor<Kernel>* memberFunctor =
   new vpr::ThreadMemberFunctor<Kernel>(this, &Kernel::controlLoop, NULL);

   //vpr::Thread* new_thread;   // I set mControlThread in Kernel::controlLoop
   //new_thread =
   new vpr::Thread(memberFunctor);
   //vprASSERT(new_thread->valid());

   vprDEBUG(vrjDBG_KERNEL,vprDBG_STATE_LVL) << "vjKernel::start: Just started control loop.  "
                                         << std::endl << vprDEBUG_FLUSH;

   return 1;
}

// Set the stop flag
// NOTE: The kernel should not actually stop until the application has been close (ie. mApp is NULL)
void Kernel::stop()
{
   mExitWaitCondVar.acquire();
   {
      mExitFlag = true;
      mExitWaitCondVar.signal();
   }
   mExitWaitCondVar.release();

   setApplication(NULL);      // Set NULL application so that the app gets closed
}

/** Blocks until the kernel exits 
*
* Waits for !mIsRunning && mExitFlag
* 
* Need both of those, because even though exit flag may be triggered
* we can not exit until the kernel control loop stops.  This is signaled by
* setting mIsRunning = false;
*/
void Kernel::waitForKernelStop()
{
   mExitWaitCondVar.acquire();
   {
      while (! (!mIsRunning && mExitFlag) )   // Wait until !mIsRunning && mExitFlag
      {
         mExitWaitCondVar.wait();
      }
   }
   mExitWaitCondVar.release();
}


/// The Kernel loop
void Kernel::controlLoop(void* nullParam)
{
   vprDEBUG(vrjDBG_KERNEL,1) << "vjKernel::controlLoop: Started.\n" << vprDEBUG_FLUSH;

   while (0 == vpr::Thread::self())
   {
      vprDEBUG(vprDBG_ALL,1) << "vjKernel: Waiting for (thread::self() != NULL)\n" << vprDEBUG_FLUSH;
      vpr::System::usleep(50);
   }
   mControlThread = (vpr::Thread*) vpr::Thread::self();

   // Do any initial configuration
   initConfig();

   // --- MAIN CONTROL LOOP -- //
   while(! (mExitFlag && (mApp == NULL)))     // While not exit flag set and don't have app. (can't exit until app is closed)
   {
       jcclTIMESTAMP (jcclPERF_ALL, "kernel/startframe");

      // Iff we have an app and a draw manager
      if((mApp != NULL) && (mDrawManager != NULL))
      {
            vprDEBUG(vrjDBG_KERNEL,5) << "vjKernel::controlLoop: mApp->preFrame()\n" << vprDEBUG_FLUSH;
         mApp->preFrame();         // PREFRAME: Do Any application pre-draw stuff
            jcclTIMESTAMP (jcclPERF_ALL, "kernel/app/preFrame()");
            vprDEBUG(vrjDBG_KERNEL,5) << "vjKernel::controlLoop: drawManager->draw()\n" << vprDEBUG_FLUSH;
         mDrawManager->draw();    // DRAW: Trigger the beginning of frame drawing
         mSoundManager->update();
            jcclTIMESTAMP (jcclPERF_ALL, "kernel/trigger Draw");
            vprDEBUG(vrjDBG_KERNEL,5) << "vjKernel::controlLoop: mApp->intraFrame()\n" << vprDEBUG_FLUSH;
         mApp->intraFrame();        // INTRA FRAME: Do computations that can be done while drawing.  This should be for next frame.
            jcclTIMESTAMP (jcclPERF_ALL, "kernel/app/intraFrame()");
            vprDEBUG(vrjDBG_KERNEL,5) << "vjKernel::controlLoop: drawManager->sync()\n" << vprDEBUG_FLUSH;
         mSoundManager->sync();
         mDrawManager->sync();    // SYNC: Block until drawing is done
            jcclTIMESTAMP (jcclPERF_ALL, "kernel/wait for draw threads");
            vprDEBUG(vrjDBG_KERNEL,5) << "vjKernel::controlLoop: mApp->postFrame()\n" << vprDEBUG_FLUSH;
         mApp->postFrame();        // POST FRAME: Do processing after drawing is complete
           jcclTIMESTAMP (jcclPERF_ALL, "kernel/app/postFrame");
      }
      else
      {
         // ??? Should we do this, or just grind up the CPU as fast as possible
         vprASSERT(NULL != mControlThread);      // If control thread is not set correctly, it will seg fault here
         vpr::Thread::yield();   // Give up CPU
      }

      // --- Stop for reconfiguration -- //
      checkForReconfig();        // Check for any reconfiguration that needs done (system or application)
      checkSignalButtons();      // Check for any pending control requests
       jcclTIMESTAMP (jcclPERF_ALL, "kernel/checkForReconfig");

         vprDEBUG(vrjDBG_KERNEL,5) << "vjKernel::controlLoop: Update Trackers\n" << vprDEBUG_FLUSH;
      getInputManager()->updateAllData();    // Update the trackers
         jcclTIMESTAMP (jcclPERF_ALL, "kernel/input/updateAllData()");
         vprDEBUG(vrjDBG_KERNEL,5) << "vjKernel::controlLoop: Update Projections\n" << vprDEBUG_FLUSH;
      updateFrameData();         // Update the projections, etc.
         jcclTIMESTAMP (jcclPERF_ALL, "kernel/updateFrameData");
   }   

   vprDEBUG(vrjDBG_KERNEL,1) << "vjKernel::controlLoop: Exiting. \n" << vprDEBUG_FLUSH;

   // Set the running status to false
   mExitWaitCondVar.acquire();
   {
      mIsRunning = false;
      mExitWaitCondVar.signal();
   }
   mExitWaitCondVar.release();     
}

// Set the application to run
// XXX: Should have protection here
void Kernel::setApplication(App* newApp)
{
   vprDEBUG(vrjDBG_KERNEL,vprDBG_CONFIG_LVL) << "vjKernel::setApplication: New application set\n" << vprDEBUG_FLUSH;
   mNewApp = newApp;
   mNewAppSet = true;
}

/**
 * Checks to see if there is reconfiguration to be done.
 * @post Any reconfiguration needed has been completed.
 * @note Can only be called by the kernel thread.
 */
void Kernel::checkForReconfig()
{
   vprASSERT(vpr::Thread::self() == mControlThread);      // ASSERT: We are being called from kernel thread


   // ---- RECONFIGURATION --- //
   jccl::ConfigManager* cfg_mgr = jccl::ConfigManager::instance();
   unsigned num_processed(0);

   do
   {
      num_processed = cfg_mgr->attemptReconfiguration();
   }
   while (num_processed > 0);

//     // If we changed the active configuration, then the environment manager needs to refresh
//     if((total_chunks_processed > 0) && (environmentManager != NULL))
//     {
//        environmentManager->sendRefresh();
//     }

   // ---- APP SWITCH ---- //
   // check for a new applications
   if(mNewAppSet)
   {
      if((mNewApp == NULL) || (mNewApp->depSatisfied()) )   // If app is NULL or dependencies satisfied
      {
         vprDEBUG(vrjDBG_KERNEL,vprDBG_CONFIG_LVL) << "vjKernel: New application dependencies: Satisfied.\n" << vprDEBUG_FLUSH;
         mNewAppSet = false;
         changeApplication(mNewApp);
      }
      else
      {
         vprDEBUG(vrjDBG_KERNEL,vprDBG_WARNING_LVL) << "vjKernel: New application dependencies: Not satisfied yet.\n" << vprDEBUG_FLUSH;
      }
   }
}

// Changes the application in use
//  If there is another app active, it has to stop that
//  application first then restart all API specif ic Managers.
//! ARGS: newApp - If NULL, stops current application
//! NOTE: This can only be called from the kernel thread
// app = NULL ==> stop draw manager and null out app
// app != NULL ==>
//             Get the draw manager needed
//             Start it
//             Give it the application
void Kernel::changeApplication(App* newApp)
{
   vprDEBUG(vrjDBG_KERNEL,1) << "vjKernel::changeApplication: Changing to:"
                           << newApp << std::endl << vprDEBUG_FLUSH;

   vprASSERT(vpr::Thread::self() == mControlThread);      // ASSERT: We are being called from kernel thread

   jccl::ConfigManager* cfg_mgr = jccl::ConfigManager::instance();

   // EXIT Previous application
   if(mApp != NULL)
   {
      cfg_mgr->removeConfigChunkHandler (mApp);
      mApp->exit();
      mApp = NULL;      // ASSERT: We have no handles to the application any more (ie. the app could be deleted)
   }

   // SET NEW APPLICATION
   if(newApp != NULL)        // We were given an app
   {
      mApp = newApp;
      DrawManager* new_draw_mgr = mApp->getDrawManager();
      vprASSERT(NULL != new_draw_mgr);

      if (new_draw_mgr != mDrawManager)      // Have NEW draw manager
      {
         stopDrawManager();                           // Stop old one
         cfg_mgr->removeConfigChunkHandler (mDrawManager);

         mDrawManager = mApp->getDrawManager();             // Get the new draw manager
         mSoundManager = mApp->getSoundManager();           // Get the new sound manager
         cfg_mgr->addConfigChunkHandler (mDrawManager);     // Tell config manager about them
         cfg_mgr->addConfigChunkHandler (mSoundManager);    // Tell config manager about them
         startDrawManager(true);                      // Start the new one
      }
      else     // SAME draw manager
      {
         startDrawManager(false);                     // Start new app
      }

      cfg_mgr->addConfigChunkHandler (mApp);
      cfg_mgr->refreshPendingList();                  // New managers, so we may be able to handle config requests now
   }
   else                 // No app, clear to NULL
   {
      stopDrawManager();
      mApp = NULL;
   }
}


/** Initialize the signal buttons for the kernel */
void Kernel::initSignalButtons()
{
   mStopKernelSignalButton.init("VJSystemStopKernel");
}

/** Check the signal buttons to see if anything has been triggered */
void Kernel::checkSignalButtons()
{
   if(mStopKernelSignalButton->getData() == gadget::Digital::ON)
   {
      vprDEBUG(vprDBG_ALL, vprDBG_CONFIG_LVL) << "StopKernelSignalButton pressed: Kernel will exit.\n" << vprDEBUG_FLUSH;
      this->stop();  // Signal kernel to stop
   }
}


//-----------------------------------------------
// Load config
// Setup Input, Display, and kernel
//!NOTE: Does initial configuration and then sends config file to configAdd
//!POST: config is init'd
//----------------------------------------------
void Kernel::initConfig()
{
   vprDEBUG_BEGIN(vrjDBG_KERNEL,3) << "vjKernel::initConfig: Setting initial config.\n" << vprDEBUG_FLUSH;

   // ---- ALLOCATE MANAGERS --- //
   //initialSetupInputManager();
   mInputManager = gadget::InputManager::instance();

   //initialSetupDisplayManager();
   mDisplayManager = DisplayManager::instance();  // Get display manager
   vprASSERT(mDisplayManager != NULL);                 // Did we get an object

   //setupEnvironmentManager();
/*
   jccl::JackalServer::instance()->addJackalControl(jccl::ConfigManager::instance());
   jccl::JackalServer::instance()->addJackalControl(jccl::PerformanceMonitor::instance());

   jccl::ConfigManager::instance()->addConfigChunkHandler(jccl::JackalServer::instance());
*/
   jccl::ConfigManager::instance()->addConfigChunkHandler(jccl::PerformanceMonitor::instance());

   //??// processPending() // Should I do this here

#if defined(VPR_OS_IRIX) || defined(VPR_OS_Linux) || defined(VPR_OS_Solaris) || \
    defined(VPR_OS_AIX) || defined(VPR_OS_FreeBSD) || defined(VPR_OS_HPUX)
   mSysFactory = SystemFactoryUNIX::instance(); // XXX: Should not be system specific
#elif defined(VPR_OS_Darwin)
   mSysFactory = SystemFactoryOSX::instance();
#elif defined(VPR_OS_Win32)
   mSysFactory = SystemFactoryWin32::instance();
#else
   //vprDEBUG(0,0) << "ERROR!: Don't know how to create System Factory!\n" << vprDEBUG_FLUSH;
   vprASSERT(false);
#endif

   // hook dynamically-reconfigurable managers up to config manager...
   jccl::ConfigManager::instance()->addConfigChunkHandler(this);
   jccl::ConfigManager::instance()->addConfigChunkHandler(mInputManager);
   jccl::ConfigManager::instance()->addConfigChunkHandler(mDisplayManager);

   vprDEBUG_END(vrjDBG_KERNEL,3) << "vjKernel::initConfig: Done.\n" << vprDEBUG_FLUSH;
}


void Kernel::updateFrameData()
{
   // When we have a draw manager, tell it to update it's projections
   mDisplayManager->updateProjections();
}


//  // -------------------------------
//  // CHUNK Handler
//  // -------------------------------
//  //: Process any pending reconfiguration that we can deal with
//  //
//  //  For all dependant managers, call process pending.
//  //  and call it on our selves
//  int Kernel::configProcessPending(bool lockIt)
//  {
//     int chunks_processed(0);     // Needs to return this value

//     ConfigManager* cfg_mgr = ConfigManager::instance();
//     if(cfg_mgr->pendingNeedsChecked())
//     {
//        vprDEBUG_BEGIN(vprDBG_ALL,vprDBG_STATE_LVL) << "vjKernel::configProcessPending: Examining pending list.\n" << vprDEBUG_FLUSH;

//        chunks_processed += jccl::ConfigChunkHandler::configProcessPending(lockIt);      // Process kernels pending chunks
//        chunks_processed += getInputManager()->configProcessPending(lockIt);
//        chunks_processed += mDisplayManager->configProcessPending(lockIt);
//        if(NULL != mSoundManager)
//           chunks_processed += mSoundManager->configProcessPending(lockIt);
//        if(NULL != mDrawManager)
//           chunks_processed += mDrawManager->configProcessPending(lockIt);              // XXX: We should not necessarily do this for all draw mgrs
//        if (NULL != environmentManager)
//           chunks_processed += environmentManager->configProcessPending(lockIt);
//        if(NULL != mApp)
//           chunks_processed += mApp->configProcessPending(lockIt);

//        vprDEBUG_CONT_END(vprDBG_ALL,vprDBG_STATE_LVL) << std::endl
//                                                     << vprDEBUG_FLUSH;
//     }
//     return chunks_processed;
//  }


bool Kernel::configCanHandle(jccl::ConfigChunkPtr chunk)
{
   std::string chunk_type = chunk->getDescToken();

   if(std::string("JugglerUser") == chunk_type)
      return true;
   else
      return false;
}

bool Kernel::configAdd(jccl::ConfigChunkPtr chunk)
{
   std::string chunk_type = chunk->getDescToken();

   vprASSERT(configCanHandle(chunk));

   if(std::string("JugglerUser") == chunk_type)
   {
      return addUser(chunk);
   }
   else
      return false;
}

bool Kernel::configRemove(jccl::ConfigChunkPtr chunk)
{
   std::string chunk_type = chunk->getDescToken();

   vprASSERT(configCanHandle(chunk));

   if(std::string("JugglerUser") == chunk_type)
   {
      return removeUser(chunk);
   }
   else
      return false;
}

/** Adds a new user to the kernel. */
bool Kernel::addUser(jccl::ConfigChunkPtr chunk)
{
   vprASSERT(chunk->getDescToken() == std::string("JugglerUser"));

   User* new_user = new User;
   bool success = new_user->config(chunk);

   if(!success)
   {
      vprDEBUG(vrjDBG_KERNEL,vprDBG_CRITICAL_LVL)
         << clrOutNORM(clrRED,"ERROR:") << "Failed to add new User: "
         << chunk->getName() << std::endl
         << vprDEBUG_FLUSH;
      delete new_user;
   }
   else
   {
      vprDEBUG(vrjDBG_KERNEL,vprDBG_STATE_LVL)
                             << "vjKernel: Added new User: "
                             << new_user->getName().c_str() << std::endl
                             << vprDEBUG_FLUSH;
      mUsers.push_back(new_user);
   }

   return success;
}

// XXX: Not implemented
bool Kernel::removeUser(jccl::ConfigChunkPtr chunk)
{
   return false;
}

// --- STARTUP ROUTINES --- //
void Kernel::loadConfigFile(std::string filename)
{
   vprDEBUG(vrjDBG_KERNEL,vprDBG_CONFIG_LVL) << "Loading config file: "
                           << filename << std::endl << vprDEBUG_FLUSH;

   jccl::ConfigChunkDB* chunk_db = new jccl::ConfigChunkDB;

   // ------- OPEN Program specified Config file ------ //
   if(filename.empty())   // We have a filename
     return;

   bool chunk_db_load_success = chunk_db->load(filename);
   if (!chunk_db_load_success)
   {
     vprDEBUG(vprDBG_ERROR,vprDBG_CRITICAL_LVL) << clrOutNORM(clrRED,"ERROR:")
        << "vjKernel::loadConfigFile: DB Load failed to load file: "
        << filename.c_str() << std::endl << vprDEBUG_FLUSH;
     exit(1);
   }

   // Put them all in pending
   jccl::ConfigManager::instance()->addPendingAdds(chunk_db);

   //vprDEBUG(vrjDBG_KERNEL,5) << "------------  Loaded Config Chunks ----------" << vprDEBUG_FLUSH;
   //vprDEBUG(vrjDBG_KERNEL,5) << (*mInitialChunkDB) << vprDEBUG_FLUSH;
}

/**
 * Loads a chunk description file.
 * @post The chunk factory can now manage chunks with the given types.
 */
void Kernel::loadChunkDescFile(std::string filename)
{
   jccl::ChunkFactory::instance()->loadDescs(filename);
}

/**
 * This starts up the Draw Manager given.
 * @post All processes and data should have been created by Draw Manager.
 */
void Kernel::startDrawManager(bool newMgr)
{
   vprASSERT((mApp != NULL) && (mDrawManager != NULL) && (mDisplayManager != NULL));

   if(newMgr)
   {
      mDrawManager->setDisplayManager(mDisplayManager);
      jccl::ConfigManager::instance()->lockPending();
      {
         mDrawManager->configProcessPending();                 // Handle any pending configuration requests BEFORE we init and start it going
      }
      jccl::ConfigManager::instance()->unlockPending();
   }
   mDrawManager->setApp(mApp);

   mApp->init();                     // Init the app
   if(newMgr)
      mDrawManager->initAPI();       // Just sets up API type stuff, possibly starts new processes
   mApp->apiInit();                  // Have app do any app-init stuff
   if(newMgr)
   {
     mDisplayManager->setDrawManager(mDrawManager);      // This can trigger the update of windows to the draw manager
   }
}

// Stop the draw manager and close it's resources, then delete it
//! POST: draw mgr resources are closed
//+       draw mgr is deleted, display manger set to NULL draw mgr
// XXX: @todo Make stop draw manager reconfigure the draw manager to close all the windows
void Kernel::stopDrawManager()
{
   if(mDrawManager != NULL)
   {
      mDrawManager->closeAPI();
      delete mDrawManager;
      mDrawManager = NULL;
      mDisplayManager->setDrawManager(NULL);
   }
}


/** Gets the Input Manager. */
gadget::InputManager* Kernel::getInputManager()
{ return mInputManager; }


User* Kernel::getUser(std::string userName)
{
   for(unsigned int i=0;i<mUsers.size();i++)
      if(userName == mUsers[i]->getName())
         return mUsers[i];

   return NULL;
}

Kernel::Kernel()
{
   mApp        = NULL;
   mNewApp     = NULL;
   mNewAppSet  = false;
   mExitFlag      = false;
   mIsRunning     = false;
   mControlThread = NULL;
   mSysFactory    = NULL;
   mInputManager  = NULL;
   mDrawManager   = NULL;
   mSoundManager  = NULL;
   mDisplayManager = NULL;

   //mInitialChunkDB = NULL;
   //mChunkDB = NULL;

   // Print out the Juggler version number when the kernel is created.
   vprDEBUG(vprDBG_ALL, 0) << std::string(strlen(VJ_VERSION) + 12, '=')
                           << std::endl << vprDEBUG_FLUSH;
   vprDEBUG(vprDBG_ALL, 0) << clrOutNORM(clrGREEN, "VR Juggler: ")
                           << clrOutNORM(clrGREEN, VJ_VERSION) << clrRESET
                           << std::endl << vprDEBUG_FLUSH;
   vprDEBUG(vprDBG_ALL, 0) << clrOutNORM(clrGREEN, "VPR: ")
                           << clrOutNORM(clrGREEN, vpr::getVersionString())
                           << clrRESET << std::endl << vprDEBUG_FLUSH;
   vprDEBUG(vprDBG_ALL, 0) << clrOutNORM(clrGREEN, "Gadgeteer: ")
                           << clrOutNORM(clrGREEN, gadget::getVersionString())
                           << clrRESET << std::endl << vprDEBUG_FLUSH;
   vprDEBUG(vprDBG_ALL, 0) << std::string(strlen(VJ_VERSION) + 12, '=')
                           << std::endl << vprDEBUG_FLUSH;

   jccl::ChunkFactory::instance()->loadDescs
       ("${VJ_BASE_DIR}/" VJ_SHARE_DIR "/data/vrj-chunks.desc");
}

};
