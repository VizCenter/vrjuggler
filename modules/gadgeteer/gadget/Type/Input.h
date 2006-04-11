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
// Base class for all vj Input devices
//
// History:
//
// Andy Himberger:    v0.1 (10/12/97) - Integeration to new libraries,
//                                      support for
//              Abilities, Instance Names, Device Names, Config Chunk
//              constructors
// Andy Himberger:    v0.0  - Inital version
////////////////////////////////////////////////////////////////////////////

#ifndef _GADGET_INPUT_H_
#define _GADGET_INPUT_H_

#include <gadget/gadgetConfig.h>
#include <vpr/vpr.h>

#include <vpr/Sync/Mutex.h>
#include <vpr/Sync/Guard.h>
#include <vpr/Thread/Thread.h>
#include <jccl/Config/ConfigChunkPtr.h>

//#include <vpr/IO/ObjectReader.h>
//#include <vpr/IO/ObjectWriter.h>
#include <gadget/RemoteInputManager/SerializableDevice.h>

// consider moving this
typedef unsigned char byte;

/** Abilities List
 *
 *  Input devices can have one or more 'Abilities'
 *  The function FDeviceSupport(ability) allows a user
 *  of a Input object to query which types it may be
 *  casted up to.
 */
/*
const unsigned int DEVICE_POSITION = 1;
const unsigned int DEVICE_GESTURE  = 2;
const unsigned int DEVICE_DIGITAL  = 4;
const unsigned int DEVICE_ANALOG   = 8;
const unsigned int DEVICE_GLOVE    = 16;
const unsigned int DEVICE_KEYBOARD = 32;
const unsigned int DEVICE_GROW1    = 64;
const unsigned int DEVICE_GROW2    = 128;
*/


namespace gadget
{

/** Input is the abstract base class that all input objects derive from.
*
*  Input is the base class for all Input Devices, all the devices are
*  therefore forced to implement the pure virtual functions of Sample,
*  StartSampling, StopSampling, and UpdateData. <br> <br>
*
*  Dummy devices can use a default constructor, but physical devices should
*  have a Constructor which takes a config chunk and calls the Input
*  constructor taking a ConfigChunkPtr <br> <br>
*
*  All Physical devices will inherit from not Input but another abstract
*  class which inherits from Input, currently there is support for
*  Positional Devices, Analog Devices, and Digital devices, each has its own
*  added pure virtual functions providing a simple and equal interface to
*  themselves.
*
* @note  We make the assumption in all devices that while UpdateData() is being
*       called, no other process will try to read the current data.
*       We can make this assumption because the whole idea of UpdateData() is
*       to bring in a current copy of the data for threads to process for a
*       frame.  Because of this, threads should not be reading data while
*       it is being updated to the most recent copy.
*/
class GADGET_CLASS_API Input : public SerializableDevice
{
public:
   /** Default Constructor
   *
   *  The default constructor is intended only for use by the DummyProxies
   *  which do not need to have their serial port and baud rate etc set up.
   * Also, initializes mThread, and active to null values
   */
   Input();

   /** Input Destructor
   *
   * Free the memory for the Instance Name and Serial Port strings if
   * allocated
   */
   virtual ~Input();

   /** Config method
   *
   *  This baselevel config will fill the base datamembers
   *  when found in the ConfigChunkPtr such as serial port, instance name
   *  and baud rate.
   */
   virtual bool config(jccl::ConfigChunkPtr c);

   /** Sample the device
   *
   *  Every input device should have a sample function, after which the
   *  device has been sampled to have new data.  (This new data is not
   *  accessable until UpdateData is called, however)
   */
   virtual int sample() = 0;

   /** Start a device sampling.
   *
   *  Start the device sampling, normally this will spawn a thread which will
   *  just repeatedly call Sample().
   *  This function should return true when it sucessfully starts,
   *      false otherwise.
   */
   virtual int startSampling() = 0;

   /* StopSampling.
   *
   *  Reverse the effects of StartSampling()
   */
   virtual int stopSampling() = 0;

   /** Update the data
   *
   *  After this function is called subsequent calls to GetData(d) will
   *  return the most recent data at the time of THIS function call.  Data is
   *  guaranteed to be valid and static until the next call to UpdateData.
   */
   virtual void updateData() = 0;

   /** Returns the string rep of the chunk type used to config this device
   * This string is used by the device factory to look up device drivers
   * based up the type of chunk it is trying to load.
   */
   static std::string getChunkType() { return std::string("Undefined"); }

   /** getInstanceName()
   *
   *  Returns the name identifying this instance of the device.
   * This is the name given to the device in it's config chunk (ie. "MyFlockOfBirds", "TheIbox", etc)
   */
   const std::string getInstanceName()
   {
      if (mInstName.empty())
      {
         return std::string("Undefined");
      }
      return mInstName;
   }
   
   virtual std::string getBaseType()
   {
      return std::string("Input");
   }

   virtual vpr::ReturnStatus writeObject(vpr::ObjectWriter* writer)
   {
      return vpr::ReturnStatus(vpr::ReturnStatus::Fail);
   }

   virtual vpr::ReturnStatus readObject(vpr::ObjectReader* reader, vpr::Uint64* delta)
   {
      return vpr::ReturnStatus(vpr::ReturnStatus::Fail);
   }

   /** Is this input device active?. */
   int isActive()
   {
      return mActive;
   }

protected:
   std::string    mPort;
   std::string    mInstName;
   vpr::Thread*   mThread;       /**< The thread being used by the driver */
   int            mActive;       /**< Is the driver active? */
   int            mBaudRate;     /**< Baud rate of the device (if it is serial device) */

   Input (const Input& o) 
      : SerializableDevice()
   {;}
   void operator= (const Input& o) {;}
};

} // end namespace

#endif   /* GADGET_INPUT_H */
