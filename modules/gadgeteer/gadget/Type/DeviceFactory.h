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

#ifndef _GADGET_DEVICE_FACTORY_H_
#define _GADGET_DEVICE_FACTORY_H_
//#pragma once

#include <gadget/gadgetConfig.h>
#include <gadget/Type/Input.h>
#include <jccl/Config/ConfigChunkPtr.h>
#include <vpr/Util/Singleton.h>

#include <vpr/Util/Debug.h>
#include <vpr/Util/Assert.h>

namespace gadget
{

/**
 * Base class for virtual construction of devices.
 * Implementations of this class are registered with the device factory
 * for each device in the system.
 */
class DeviceConstructorBase
{
public:
   /**
    * Constructor.
    * @post Device is registered.
    */
   DeviceConstructorBase() {;}

   /** Creates the device. */
   virtual Input* createDevice(jccl::ConfigChunkPtr chunk)
   {
      vprDEBUG(vprDBG_ALL,0) << "ERROR: DeviceConstructorBase::createDevice: Should never be called" << vprDEBUG_FLUSH;
      return NULL;
   }

   /** Gets the string desc of the type of chunk we can create. */
   virtual std::string    getChunkType()
   { return std::string("BaseConstructor: Invalid type"); }
};

/**
 * Object used for creating devices.
 * @note Singleton
 */
class GADGET_CLASS_API DeviceFactory
{
private:
   // Singleton so must be private
   DeviceFactory()
   {
      mConstructors = std::vector<DeviceConstructorBase*>(0);
      vprASSERT(mConstructors.size() == 0);
   }

   // This should be replaced with device plugins.
   /**
    * @post Devices are loaded that the system knows about.
    */
   void hackLoadKnownDevices();

public:
   void registerDevice(DeviceConstructorBase* constructor);

   /**
    * Queries if the factory knows about the given device.
    * @pre chunk != NULL, chunk is a valid chunk.
    * @param chunk The chunk we are requesting about knowledge to create.
    * @return true if the factory knows how to create the device; false if not.
    */
   bool recognizeDevice(jccl::ConfigChunkPtr chunk);

   /**
    * Loads the specified device.
    * @pre recognizeDevice(chunk) == true
    * @param chunk The specification of the device to load.
    * @return NULL is returned if the device failed to load.
    *         Otherwise, a pointer to the loaded device is returned.
    */
   Input* loadDevice(jccl::ConfigChunkPtr chunk);

private:
   /**
    * Finds a constructor for the given device type.
    * @return -1 is returned if the constructor is not found.
    *         Otherwise, the index of the constructor is returned.
    */
   int   findConstructor(jccl::ConfigChunkPtr chunk);

   void debugDump();


private:
   std::vector<DeviceConstructorBase*> mConstructors;  /**<  List of the device constructors */

   vprSingletonHeaderWithInitFunc(DeviceFactory, hackLoadKnownDevices);
};

template <class DEV>
class DeviceConstructor : public DeviceConstructorBase
{
public:
   DeviceConstructor()
   {
      vprASSERT(DeviceFactory::instance() != NULL);
      DeviceFactory::instance()->registerDevice(this);
   }

   Input* createDevice(jccl::ConfigChunkPtr chunk)
   {
      DEV* new_dev = new DEV;
      bool success = new_dev->config(chunk);
      if(success)
      {
         return new_dev;
      }
      else
      {
         delete new_dev;
         return NULL;
      }
   }

   virtual std::string getChunkType()
   { return DEV::getChunkType(); }
};

} // end namespace gadget

#endif
