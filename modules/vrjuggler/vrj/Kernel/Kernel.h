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

#ifndef _VRJ_KERNEL_H_
#define _VRJ_KERNEL_H_
//#pragma once

#include <vrj/vrjConfig.h>

#include <vpr/Util/Singleton.h>

//#include <vrj/Kernel/SystemFactory.h>
#include <vpr/Sync/CondVar.h>

#include <gadget/Type/DigitalInterface.h>

namespace gadget
{
   class InputManager;
}

namespace cluster
{
   class ClusterManager;
}

namespace vpr
{
   class BaseThread;
}

namespace vrj
{
class DisplayManager;
class DrawManager;
class SoundManager;
class User;
class App;


/** Main control class for all vj applications.
 *
 * Takes care of all initialization and object creation
 * for the system.
 * This class is the only class that MUST be instantiated
 * for all applications.
 */
class VJ_CLASS_API Kernel : public jccl::ConfigChunkHandler
{
public:

   /** Start the Kernel running
    * Spawn kernel thread
    */
   int start();

   /** Stop the kernel control loop
   * If there is an application set, then it will be closed first
   * @post The kernel exits and the Juggler system shuts down
   */
   void stop();

   /** Return status of kernel */
   bool isRunning() const
   { return mIsRunning; }

   /** Blocks until the kernel exits */
   void waitForKernelStop();

   /** Load initial configuration data for the managers
   * @post InputManager, DisplayManager, and kernel
   *       Config files loaded and handed to configAdd
   */
   void initConfig();

   /** The Kernel loop */
   void controlLoop(void* nullParam);

   /** Set the application object for the Kernel to deal with
   *  If there is another app active, it has to stop that
   *  application first then restart all API specific Managers.
   * @param newApp - If NULL, stops current application
   */
   void setApplication(vrj::App* newApp);

   /** Load configuration data for Kernel
   * @post Config data has been read into initial buffer
   */
   void loadConfigFile(const char* filename)
   {
      std::string filename_str = std::string(filename);
      loadConfigFile(filename_str);
   }

   /** Load configuration data for Kernel
   * @post Config data has been read into initial buffer
   */
   void loadConfigFile(std::string filename);

   /** Load a chunk description file
   * @post The chunk factory can now manage chunks with the given types
   */
   void loadChunkDescFile(std::string filename);


protected:  // -- CHUNK HANDLER
   /** Can the handler handle the given chunk?
   * @return true - Can handle it
   *          false - Can't handle it
   */
   virtual bool configCanHandle(jccl::ConfigChunkPtr chunk);

   /** Process any pending reconfiguration that we can deal with
   *
   *  Process pending reconfiguration of the kernel and
   *  it's dependant managers (basically all of them
   *  that don't have a control thread)
   *
   *     // It just calls process pending for dependant processes
   *     virtual int configProcessPending(bool lockIt = true);
   */

protected:  // -- CHUNK HANDLER
   /** Add the chunk to the configuration
   * @pre    configCanHandle(chunk) == true
   * @return success
   */
   virtual bool configAdd(jccl::ConfigChunkPtr chunk);

   /** Remove the chunk from the current configuration
   * @pre    configCanHandle(chunk) == true
   * @return success
   */
   virtual bool configRemove(jccl::ConfigChunkPtr chunk);

protected:  // Local config functions
   /** Add a User to the system */
   bool addUser(jccl::ConfigChunkPtr chunk);

   /** Remove a User from the system
   * @note Currently not implemented
   */
   bool removeUser(jccl::ConfigChunkPtr chunk);

protected:
   /** Updates any data that needs updated once a frame (Trackers, etc.)
    * @post All tracker data is ready for next frame
    */
   void updateFrameData();

   /** Checks to see if there is reconfiguration to be done
    * @post Any reconfiguration needed has been completed
    * @note Can only be called by the kernel thread
    */
   void checkForReconfig();

   /** Changes the application in use
    * If there is another app active, it has to stop that
    *  application first then restart all API specific Managers.
    * @param   newApp - If NULL, stops current application
    * @note    This can only be called from the kernel thread
    */
   void changeApplication(vrj::App* newApp);

   /** Initialize the signal buttons for the kernel */
   void initSignalButtons();

   /** Check the signal buttons to see if anything has been triggered */
   void checkSignalButtons();

protected:      // --- DRAW MGR ROUTINES --- //
   /** Starts the draw manager running
   * Calls the app callbacks for the draw manager
   * param newMgr - Is this a new manager, or the same one
   */
   void startDrawManager(bool newMgr);

   /** Stop the draw manager and close it's resources, then delete it
   * @post draw mgr resources are closed
   *       draw mgr is deleted, display manger set to NULL draw mgr
   */
   void stopDrawManager();

public:      // Global "get" interface

   /** Get the Input Manager. */
   gadget::InputManager* getInputManager();

   /** Get the user associated with given name.
    * @return NULL if not found.
    */
   vrj::User* getUser(const std::string& userName);

   /** Get a list of the users back. */
   std::vector<vrj::User*> getUsers()
   { return mUsers; }

   const vpr::BaseThread* getThread()
   { return mControlThread; }

protected:
   vrj::App*      mApp;                         /**< The current active app object */
   vrj::App*      mNewApp;                      /**< New application to set */
   bool           mNewAppSet;                   /**< Flag to notify that a new application should be set */

   bool               mIsRunning;               /**< Flag for wether the kernel is currently running */
   bool               mExitFlag;                /**< Set true when the kernel should exit */
   vpr::BaseThread*   mControlThread;           /**< The thread in control of me */
   vpr::CondVar       mExitWaitCondVar;         /**< Cond var for waiting for exit */

   /// Factories and Managers
   gadget::InputManager*      mInputManager;          /**< The input manager for the system  */
   DrawManager*               mDrawManager;           /**< The Draw Manager we are currently using */
   SoundManager*              mSoundManager;          /**< The Audio Manager we are currently using  */
   DisplayManager*            mDisplayManager;        /**< The Display Manager we are currently using */
   cluster::ClusterManager*   mClusterManager;        /**< The Cluster Manager for the system*/

   /// Multi-user information
   std::vector<vrj::User*>   mUsers;         /** A list of user objects in system */

   // Control "signals" from input interfaces
   gadget::DigitalInterface   mStopKernelSignalButton;

   // ----------------------- //
   // --- SINGLETON STUFF --- //
   // ----------------------- //
protected:
   /** Constructor:  Hidden, so no instantiation is allowed. */
   Kernel();

   virtual ~Kernel()
   {;}

   vprSingletonHeader( Kernel );
};

} // end namespace

#endif
