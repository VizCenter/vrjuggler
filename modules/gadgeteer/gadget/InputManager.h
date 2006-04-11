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


/////////////////////////////////////////////////////////////////////////////
//
// Group manager for vj input devices and proxies
//
////////////////////////////////////////////////////////////////////////////
#ifndef GADGET_INPUT_MANAGER_H
#define GADGET_INPUT_MANAGER_H

#include <gadget/gadgetConfig.h>

#include <vpr/Util/Singleton.h>
#include <jccl/RTRC/ConfigChunkHandler.h>
#include <map>


namespace gadget
{

// Proxies
class Proxy;
class Input;
class RemoteInputManager;

/**
 * The Input Manager holds an manages all Gadgeteer Input devices.
 *
 * The Input Manager, handles all the details behind organizing the input
 * devices in the library.  It provides an API for adding devices by their
 * JCCL config chunk and deleting them by their chunk (or just their string
 * name).
 *
 * Proxies are used to abstract away from the devices so any type and number
 * of devices may be accessed in the same way.  These proxies are also
 * set up by configChunks and should be accessed by number, rather than name
 * (for speed).
 */
class GADGET_CLASS_API InputManager : public jccl::ConfigChunkHandler
{
   vprSingletonHeader( InputManager );    // Make it a singleton
public:
   InputManager();
   virtual ~InputManager();

   friend GADGET_API(std::ostream&) operator<<(std::ostream& out,
                                           InputManager& iMgr);

 //---------------------------//
 //      CONFIG               //
 //---------------------------//
   /**
    * Adds the chunk to the configuration.
    * @pre configCanHandle(chunk) == true
    * @return success
    */
   bool configAdd(jccl::ConfigChunkPtr chunk);

   /**
    * Removes the chunk from the current configuration.
    * @pre configCanHandle(chunk) == true
    * @post (chunk is proxy) ==> (returns == false)<br>
    *       (chunk is device) ==> (devices is removed && proxies are stupified)<br>
    *       (chunk is proxyAlias) ==> (proxyAlias is removed && devInterfaces.refreshAll())
    * @return success
    */
   bool configRemove(jccl::ConfigChunkPtr chunk);

   /**
    * Can the handler handle the given chunk?
    *
    * @return true if the handler can handle it, false otherwise.
    */
   bool configCanHandle(jccl::ConfigChunkPtr chunk);

   jccl::ConfigChunkPtr getDisplaySystemChunk();

// MOVE FOR RIM's USE private:
   /**
    * Loads the device for the given chunk.
    * @return true if the device was configured and added
    */
   bool configureDevice(jccl::ConfigChunkPtr chunk);

private:
   /**
    * Loads the Proxy for the given chunk.
    * @return true if the proxy was configured and added.
    */
   bool configureProxy(jccl::ConfigChunkPtr chunk);

   /**
    * Removes the device associated with the given chunk.
    * @return true if the device was removed.
    */
   bool removeDevice(jccl::ConfigChunkPtr chunk);


   // ------------------------------- //
   //        DEVICE API               //
   // ------------------------------- //
public:
   /**
    * Updates the data in all the devices.
    * This calls updateData() on all the devices in the device table.
    * It also updates all proxies.
    *
    * @modifies Instances pointed to by device table.
    */
   void updateAllData();

public:
   /**
    * Returns an Input pointer to a named device.
    *
    * @return NULL if the device is not found.
    */
   Input* getDevice(std::string deviceName);

   /**
    * Adds a device to InputManager.
    * Adds the devPtr to the device table.  devPtr should not already be in
    * the array.
    *
    * @modifies self
    * @post mDevTable' = mDevTable \/ devPtr
    *
    * @return false if device addition fails.
    */
   bool addDevice(Input* devPtr);

private:
   /**
    * Removes a device from the InputManager.
    * Remove the named device from the device table.
    *
    * @modifies self
    * @post mDevTable[devNum]' = NULL
    *
    * @return true on success.
    */
   bool removeDevice(std::string mInstName);
   bool removeDevice(const Input* devPtr);

   /*********************************************************
    *          PROXIES                                      *
    *********************************************************/
public:

   /**
    * Adds a proxy to the proxy table.
    *
    * @return true if the proxy was added correctly.
    */
   bool addProxy(std::string proxyName, Proxy* proxy);

   /**
    * Gets a proxy for the given proxy name (or alias).
    *
    * @return NULL if the proxy was not found.
    */
   Proxy* getProxy(std::string proxyName);

   /**
    * Refreshes all the proxies to have them update the device at which they
    * point.
    */
   void refreshAllProxies();

protected:
   bool removeProxy(std::string proxyName);
   bool removeProxy(jccl::ConfigChunkPtr chunk);

protected:
   typedef std::map<std::string,Input*> tDevTableType;

   tDevTableType                        mDevTable;
   std::map<std::string, Proxy*>        mProxyTable;    /**< list of proxies in the system */
   std::map<std::string, std::string>   mProxyAliases;  /**< List of alias names for proxies */
   
   // The mProxyAlias table serves as a secondary lookup for proxies.  ie. if
   // the proxy name is not found in mProxyTable, then search mProxyAliases
   // for it.

   jccl::ConfigChunkPtr    mDisplaySystemChunk;    /**< Config chunk for the displaySystem */
	std::string localHostName;
private:
   /** Function to configure the proxy Alias array. */
   bool configureProxyAlias(jccl::ConfigChunkPtr chunk);

   /** Removes a proxy alias. */
   bool removeProxyAlias(jccl::ConfigChunkPtr chunk);

   /** Adds a proxy alias. */
   void addProxyAlias(std::string alias_name, std::string proxy_name);

   /*********************************************************
   *          RemoteInputManger                             *
   *********************************************************/

public:
   RemoteInputManager* getRemoteInputManager(){ return mRemoteInputManager; }

protected:
   bool configureRemoteInputManager(jccl::ConfigChunkPtr chunk);
   bool configureRemoteConnection(jccl::ConfigChunkPtr chunk);

private:
   RemoteInputManager* mRemoteInputManager;
};

/** Writes out the status of the Input Manager. */
GADGET_API(std::ostream&) operator<<(std::ostream& out, InputManager& iMgr);

} // end namespace

#endif
