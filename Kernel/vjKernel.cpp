/*
 *  File:	    $RCSfile$
 *  Date modified:  $Date$
 *  Version:	    $Revision$
 *
 *
 *                                VR Juggler
 *                                    by
 *                              Allen Bierbaum
 *                             Christopher Just
 *                             Patrick Hartling
 *                            Carolina Cruz-Neira
 *                               Albert Baker
 *
 *                         Copyright  - 1997,1998,1999
 *                Iowa State University Research Foundation, Inc.
 *                            All Rights Reserved
 */


#include <vjConfig.h>
#include <string.h>
#include <Kernel/vjKernel.h>
#include <Config/vjChunkFactory.h>

// Initailize Statics
vjKernel* vjKernel::_instance = NULL;

//: Start the Kernel loop running
int vjKernel::start()
{
   if(mControlThread != NULL) // Have already started
   {
      vjDEBUG(vjDBG_ERROR,0) << "ERROR: vjKernel::start called when kernel already running\n" << vjDEBUG_FLUSH;
      vjASSERT(false);
      exit(0);
   }

   // Create a new thread to handle the control
   vjThreadMemberFunctor<vjKernel>* memberFunctor =
   new vjThreadMemberFunctor<vjKernel>(this, &vjKernel::controlLoop, NULL);

   mControlThread = new vjThread(memberFunctor, 0);

   vjDEBUG(vjDBG_KERNEL,1) << "vjKernel::start: Just started control loop.  " << mControlThread << endl << vjDEBUG_FLUSH;

   return 1;
}

/// The Kernel loop
void vjKernel::controlLoop(void* nullParam)
{
   vjDEBUG(vjDBG_KERNEL,1) << "vjKernel::controlLoop: Started.\n" << vjDEBUG_FLUSH;
   vjTimeStamp::initialize();
   // Do any initial configuration
   initConfig();

   // setup performance buffer
   perfBuffer = new vjPerfDataBuffer ("Kernel loop", 500, 8);
   environmentManager->addPerfDataBuffer (perfBuffer);

   //while(!Exit)
   while (1)
   {
      // Iff we have an app and a draw manager
      if((mApp != NULL) && (mDrawManager != NULL))
      {
            vjDEBUG(vjDBG_KERNEL,5) << "vjKernel::controlLoop: mApp->preDraw()\n" << vjDEBUG_FLUSH;
         mApp->preDraw();         // PREDRAW: Do Any application pre-draw stuff
            perfBuffer->set (0);
            vjDEBUG(vjDBG_KERNEL,5) << "vjKernel::controlLoop: drawManager->draw()\n" << vjDEBUG_FLUSH;
         mDrawManager->draw();    // DRAW: Trigger the beginning of frame drawing
            perfBuffer->set (1);
            vjDEBUG(vjDBG_KERNEL,5) << "vjKernel::controlLoop: mApp->postDraw\n" << vjDEBUG_FLUSH;
         mApp->postDraw();        // POST DRAW: Do computations that can be done while drawing.  This should be for next frame.
            perfBuffer->set (2);
            vjDEBUG(vjDBG_KERNEL,5) << "vjKernel::controlLoop: drawManager->sync()\n" << vjDEBUG_FLUSH;
         mDrawManager->sync();    // SYNC: Block until drawing is done
            perfBuffer->set (3);
            vjDEBUG(vjDBG_KERNEL,5) << "vjKernel::controlLoop: mApp->postSync()\n" << vjDEBUG_FLUSH;
         mApp->postSync();        // POST SYNC: Do processing after drawing is complete
            perfBuffer->set (4);
      }
      else
      {
         // ??? Should we do this, or just grind up the CPU as fast as possible
         mControlThread->yield();   // Give up CPU
      }

      checkForReconfig();        // Check for any reconfiguration that needs done

      perfBuffer->set(5);

         vjDEBUG(vjDBG_KERNEL,5) << "vjKernel::controlLoop: Update Trackers\n" << vjDEBUG_FLUSH;
      getInputManager()->updateAllData();    // Update the trackers
         perfBuffer->set(6);
         vjDEBUG(vjDBG_KERNEL,5) << "vjKernel::controlLoop: Update Projections\n" << vjDEBUG_FLUSH;
      updateFrameData();         // Update the projections, etc.
         perfBuffer->set(7);
   }
}

// Set the application to run
// XXX: Should have protection here
void vjKernel::setApplication(vjApp* _app)
{
   mNewApp = _app;
   mNewAppSet = true;
}


//: Checks to see if there is reconfiguration to be done
//! POST: Any reconfiguration needed has been completed
//! NOTE: Can only be called by the kernel thread
void vjKernel::checkForReconfig()
{
   vjASSERT(vjThread::self() == mControlThread);      // ASSERT: We are being called from kernel thread

   // Check for application reconfiguration
   if(mNewAppSet)
   {
      mNewAppSet = false;
      changeApplication(mNewApp);
   }

   // Sync should be here for Kernel changes from
   // the environment manager
   mRuntimeConfigSema.release();
      // This is the time that reconfig can happen
   mRuntimeConfigSema.acquire();
}

// Changes the application in use
//  If there is another app active, it has to stop that
//  application first then restart all API specific Managers.
//! ARGS: _app - If NULL, stops current application
//! NOTE: This can only be called from the kernel thread
// app = NULL ==> stop dra manager and null out app
// app != NULL ==>
//             Get the draw manager needed
//             Start it
//             Give it the application
void vjKernel::changeApplication(vjApp* _app)
{
   vjDEBUG(vjDBG_KERNEL,1) << "vjKernel::changeApplication: Changing to:" << _app << endl << vjDEBUG_FLUSH;

   vjASSERT(vjThread::self() == mControlThread);      // ASSERT: We are being called from kernel thread

   // EXIT Previous application
   if(mApp != NULL)
      mApp->exit();

   // SET NEW APPLICATION
   if(_app != NULL)        // We were given an app
   {
      mApp = _app;
      vjDrawManager* new_draw_mgr = mApp->getDrawManager();
      vjASSERT(NULL != new_draw_mgr);

      if (new_draw_mgr != mDrawManager)      // Have NEW draw manager
      {
         stopDrawManager();                           // Stop old one
         mDrawManager = mApp->getDrawManager();       // Get the new one
         startDrawManager(true);                      // Start the new one
      }
      else     // SAME draw manager
      {
         startDrawManager(false);                     // Start new app
      }
   }
   else                 // No app, clear to NULL
   {
      stopDrawManager();
      mApp = NULL;
   }
}



//-----------------------------------------------
// Initialize Shared Memory
// Load config
// Setup Input, Display, and kernel
//!NOTE: Does initial configuration and then sends config file to configAdd
//!POST: Shared Memory Initialized
//----------------------------------------------
void vjKernel::initConfig()
{
   vjDEBUG_BEGIN(vjDBG_KERNEL,3) << "vjKernel::initConfig: Setting initial config.\n" << vjDEBUG_FLUSH;

   // --- CREATE SHARED MEMORY --- //
   vjSharedPool::init();         // Try to init the pool stuff
   sharedMemPool = new vjSharedPool(1024*1024);      // XXX: should not be system specific


   // Setup initial environments.
   initialSetupInputManager();
   initialSetupDisplayManager();
   setupEnvironmentManager();

   configAdd(mInitialChunkDB, false);       // Setup the configuration, don't guard since we are not running the loop yet

#ifdef VJ_OS_SGI
   mSysFactory = vjSGISystemFactory::instance(); // XXX: Should not be system specific
#else
#ifdef VJ_OS_Linux
   mSysFactory = vjSGISystemFactory::instance(); // HACK - this could be trouble, using SGI factory
#else
   vjDEBUG (vjDBG_ERROR,0) << "ERROR!: Don't know how to create System Factory!\n" << vjDEBUG_FLUSH;
   vjASSERT(false);
#endif
#endif

   vjDEBUG_END(vjDBG_KERNEL,3) << "vjKernel::initConfig: Done.\n" << vjDEBUG_FLUSH;
}


void vjKernel::updateFrameData()
{
   // When we have a draw manager, tell it to update it's projections
   mDisplayManager->updateProjections();
}

//---------------------------------
// Config routines
//---------------------------------
void vjKernel::configAdd(vjConfigChunkDB* chunkDB, bool guarded)
{
   if(guarded)
      mRuntimeConfigSema.acquire();

   vjDEBUG_BEGIN(vjDBG_KERNEL,1) << "vjKernel: configAdd: Adding chunks.\n" << vjDEBUG_FLUSH;

   // Dependency sort the items
   int dep_result = chunkDB->dependencySort(getChunkDB());

   // If sort fails, exit with error
   if(dep_result == -1)
   {
      vjDEBUG(vjDBG_ERROR,0) << "vjKernel::configAdd: ERROR: Dependency sort failed. Aborting add.\n" << vjDEBUG_FLUSH;
      return;
   }

   // Get sorted list of chunks to add
   std::vector<vjConfigChunk*> chunks = chunkDB->getChunks();

   // For each element in chunk list
   for(int i=0;i<chunks.size();i++)
   {
      bool added_chunk = false;        // Flag: true - chunk was added

      vjDEBUG(vjDBG_KERNEL,3) << "vjKernel::configAdd: chunk: " << chunks[i]->getProperty("name") << endl << vjDEBUG_FLUSH;

      // Find manager to handle them
      if(this->configKernelHandle(chunks[i]))            // Kernel
         added_chunk = this->configKernelAdd(chunks[i]);
      if(getInputManager()->configCanHandle(chunks[i]))  // inputMgr
         added_chunk = getInputManager()->configAdd(chunks[i]);
      if(mDisplayManager->configCanHandle(chunks[i]))     // displayMgr
         added_chunk = mDisplayManager->configAdd(chunks[i]);
      if((mDrawManager != NULL) && (mDrawManager->configCanHandle(chunks[i])))   // drawMgr
         added_chunk = mDrawManager->configAdd(chunks[i]);
      if(environmentManager->configCanHandle(chunks[i]))                   // envMgr
	  added_chunk = environmentManager->configAdd(chunks[i]);
      if((mApp != NULL) && (mApp->configCanHandle(chunks[i])))                // App
         added_chunk = mApp->configAdd(chunks[i]);


      // --- Check for adding to active config --- //
      if(added_chunk)      // if added => add to config database
      {
         vjASSERT(mChunkDB != NULL);
         mChunkDB->addChunk(chunks[i]);
         int num_chunks = mChunkDB->getChunks().size();
         vjDEBUG(vjDBG_KERNEL,1) << "vjKernel::configAdd: Added chunk: " << chunks[i]->getProperty("name") << ", Now have " << num_chunks << " chunks.\n" << vjDEBUG_FLUSH;
      }
      else                 // Else: Give unrecognized error
      {
         vjDEBUG(vjDBG_KERNEL,0) << "vjKernel::configAdd: Unrecognized chunk.\n"
                    << "   type: " << chunks[i]->getType() << endl << vjDEBUG_FLUSH;
      }
   }

   // Dump status of the managers after reconfig
   vjDEBUG(vjDBG_KERNEL,1) << (*getInputManager()) << endl << vjDEBUG_FLUSH;

   if(guarded)
      mRuntimeConfigSema.release();

   // Tell the environment manager to refresh
   environmentManager->sendRefresh();

   vjDEBUG_END(vjDBG_KERNEL,1) << "vjKernel: configAdd: Done adding\n\n" << vjDEBUG_FLUSH;
}

void vjKernel::configRemove(vjConfigChunkDB* chunkDB)
{
vjGuard<vjSemaphore> runtimeSem(mRuntimeConfigSema);

    vjDEBUG_BEGIN(vjDBG_KERNEL,1) << "vjKernel: configRemove: Removing chunks.\n" << vjDEBUG_FLUSH;

   //XXX: Should do some dependency checking

   // Get list of chunks to remove
   std::vector<vjConfigChunk*> chunks = chunkDB->getChunks();

   // For each element in chunk list
   for(int i=0;i<chunks.size();i++)
   {
      bool removed_chunk = false;        // Flag: true - chunk was removed

      vjDEBUG(vjDBG_KERNEL,1) << "vjKernel::configRemove: chunk: " << chunks[i]->getProperty("name") << endl << vjDEBUG_FLUSH;

      // Find manager to handle them
      if(this->configKernelHandle(chunks[i]))                                 // Kernel
         removed_chunk = this->configKernelRemove(chunks[i]);
      if(getInputManager()->configCanHandle(chunks[i]))                          // inputMgr
         removed_chunk = getInputManager()->configRemove(chunks[i]);
      if(mDisplayManager->configCanHandle(chunks[i]))                            // displayMgr
         removed_chunk = mDisplayManager->configRemove(chunks[i]);
      if((mDrawManager != NULL) && (mDrawManager->configCanHandle(chunks[i])))   // drawMgr
         removed_chunk = mDrawManager->configRemove(chunks[i]);
      if(environmentManager->configCanHandle(chunks[i]))                      // envMgr
	      removed_chunk = environmentManager->configRemove(chunks[i]);
      if((mApp != NULL) && (mApp->configCanHandle(chunks[i])))                // App
         removed_chunk = mApp->configRemove(chunks[i]);


      // --- Check for removal from active config --- //
      if(removed_chunk)      // if removed => remove from config database
      {
         vjASSERT(mChunkDB != NULL);
         mChunkDB->removeNamed(chunks[i]->getProperty("name"));
         int num_chunks = mChunkDB->getChunks().size();
         vjDEBUG(vjDBG_KERNEL,3) << "vjKernel::configRemove: Removed chunk: Now have " << num_chunks << " chunks.\n" << vjDEBUG_FLUSH;
      }
      else                 // Else: Give unrecognized error
      {
         vjDEBUG(vjDBG_KERNEL,0) << "vjKernel::configRemove: Unrecognized chunk.\n"
                    << "   type: " << chunks[i]->getType() << endl << vjDEBUG_FLUSH;
      }
   }

   // Dump status of managers after teh reconfiguration
   vjDEBUG(vjDBG_ALL,1) << (*getInputManager()) << endl << vjDEBUG_FLUSH;

   vjDEBUG_END(vjDBG_KERNEL,1) << "vjKernel: configRemove: Exiting.\n" << vjDEBUG_FLUSH;

   // Tell the environment manager to refresh
   environmentManager->sendRefresh();
}


// -------------------------------
// Config chunks local to kernel
// -------------------------------
bool vjKernel::configKernelHandle(vjConfigChunk* chunk)
{
   std::string chunk_type = (std::string)chunk->getType();

   if(std::string("JugglerUser") == chunk_type)
      return true;
   else
      return false;
}

bool vjKernel::configKernelAdd(vjConfigChunk* chunk)
{
   std::string chunk_type = (std::string)chunk->getType();

   vjASSERT(configKernelHandle(chunk));

   if(std::string("JugglerUser") == chunk_type)
   {
      vjUser* new_user = new vjUser;
      bool success = new_user->config(chunk);
      if(!success)
         delete new_user;
      else
         mUsers.push_back(new_user);

      return success;
   }
   else
      return false;
}

bool vjKernel::configKernelRemove(vjConfigChunk* chunk)
{
   return false;
}


// --- STARTUP ROUTINES --- //
void vjKernel::loadConfigFile(std::string filename)
{
   vjDEBUG(vjDBG_KERNEL,1) << "   vjKernel::loadConfigFile: " << filename << endl << vjDEBUG_FLUSH;

   // ------ OPEN chunksDesc file ----- //
   char* vj_base_dir = getenv("VJ_BASE_DIR");
   if(vj_base_dir == NULL)
   {
      vjDEBUG(vjDBG_ERROR,0) << "vjKernel::loadConfig: Env var VJ_BASE_DIR not defined.\n" << vjDEBUG_FLUSH;
      exit(1);
   }

   if (NULL == mConfigDesc)
   {
      char chunk_desc_file[250];
      strcpy(chunk_desc_file, vj_base_dir);
      strcat(chunk_desc_file, "/Data/chunksDesc");

      mConfigDesc = new vjChunkDescDB;
      vjChunkFactory::setChunkDescDB (mConfigDesc);
      if (!mConfigDesc->load(chunk_desc_file))
      {
         vjDEBUG(vjDBG_ERROR,0) << "ERROR: vjKernel::loadConfig: Config Desc failed to load file: " << endl << vjDEBUG_FLUSH;
         exit(1);
      }
   }

   // Create chunk Data bases
   if(NULL == mInitialChunkDB)
      mInitialChunkDB = new vjConfigChunkDB();      // Create config database

   if(NULL == mChunkDB)
      mChunkDB = new vjConfigChunkDB();      // Create config database



   // ------- OPEN Program specified Config file ------ //
   if(!filename.empty())   // We have a filename
   {
      if (!mInitialChunkDB->load(filename.c_str()))
      {
         vjDEBUG(vjDBG_ERROR,0) << "ERROR: vjKernel::loadConfig: DB Load failed to load file: " << filename << endl << vjDEBUG_FLUSH;
         exit(1);
      }
   }

   vjDEBUG(vjDBG_KERNEL,5) << "------------  Loaded Config Chunks ----------" << vjDEBUG_FLUSH;
   vjDEBUG(vjDBG_KERNEL,5) << (*mInitialChunkDB) << vjDEBUG_FLUSH;
}


void vjKernel::initialSetupInputManager()
{
   mInputManager = new (sharedMemPool) vjInputManager;
   mInputManager->configureInitial(mInitialChunkDB);
}


void vjKernel::initialSetupDisplayManager()
{
   mDisplayManager = vjDisplayManager::instance();  // Get display manager
   vjASSERT(mDisplayManager != NULL);                 // Did we get an object
}


// This starts up the draw manager given
//!POST: All processes and data should have been created by draw manager
void vjKernel::startDrawManager(bool newMgr)
{
   vjASSERT((mApp != NULL) && (mDrawManager != NULL) && (mDisplayManager != NULL));

   if(newMgr)
   {
      mDrawManager->configInitial(mInitialChunkDB);     // Give it the chunk DB to extract API specific info
      mDrawManager->setDisplayManager(mDisplayManager);
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
void vjKernel::stopDrawManager()
{
   if(mDrawManager != NULL)
   {
      mDrawManager->closeAPI();
      delete mDrawManager;
      mDrawManager = NULL;
      mDisplayManager->setDrawManager(NULL);
   }
}


void vjKernel::setupEnvironmentManager()
{
   environmentManager = new vjEnvironmentManager();
}

vjUser* vjKernel::getUser(std::string userName)
{
   for(int i=0;i<mUsers.size();i++)
      if(userName == mUsers[i]->getName())
         return mUsers[i];

   return NULL;
}


