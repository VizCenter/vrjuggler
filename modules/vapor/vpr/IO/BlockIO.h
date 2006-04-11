/****************** <VPR heading BEGIN do not edit this line> *****************
 *
 * VR Juggler Portable Runtime
 *
 * Original Authors:
 *   Allen Bierbaum, Patrick Hartling, Kevin Meinert, Carolina Cruz-Neira
 *
 * -----------------------------------------------------------------
 * File:          $RCSfile$
 * Date modified: $Date$
 * Version:       $Revision$
 * -----------------------------------------------------------------
 *
 ****************** <VPR heading END do not edit this line> ******************/

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
 *************** <auto-copyright.pl END do not edit this line> ***************/

#ifndef _VPR_BLOCK_IO_H_
#define _VPR_BLOCK_IO_H_

#include <vpr/vprConfig.h>

#include <vector>
#include <string>
#include <string.h>

#include <vpr/Util/Assert.h>
#include <vpr/Util/ReturnStatus.h>
#include <vpr/Util/Interval.h>
#include <vpr/IO/IOSys.h>
#include <vpr/IO/Selector.h>

#include <vpr/IO/Stats/BaseIOStatsStrategy.h>


namespace vpr
{

/**
 * Block-style (as opposed to streaming) I/O interface.
 */
class VPR_CLASS_API BlockIO
{
public:
   /**
    * Gets the name of this I/O device.  The significance of the name depends
    * on the specific device type.
    *
    * @pre None.
    * @post A constant reference to the <code>mName</code> object is
    *       returned to the caller.
    *
    * @return An object containing the name of this device.
    */
   virtual const std::string& getName() const
   {
      return mName;
   }

   /**
    * Opens the I/O device.
    *
    * @pre The device is not already open.
    * @post An attempt is made to open the I/O device.  The resulting status
    *       is returned to the caller.  If the I/O device is opened,
    *       <code>mOpen</code> is set to true.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the device is opened
    *         successfully.<br>
    *         vpr::ReturnStatus::Fail is returned otherwise.
    */
   virtual ReturnStatus open() = 0;

   /**
    * Closes the I/O device.
    *
    * @pre The device is open.
    * @post An attempt is made to close the I/O device.  The resulting status
    *       is returned to the caller.  If the I/O device is closed,
    *       <code>mOpen</code> is set to false.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the device is closed
    *         successfully.<br>
    *         vpr::ReturnStatus::Fail is returned otherwise.
    */
   virtual ReturnStatus close() = 0;

   /**
    * Gets the open state of this I/O device.
    *
    * @pre None.
    * @post The boolean value in mOpen is returned to the caller.
    *
    * @return <code>true</code> is returned if the device is open;
    *         <code>false</code> otherwise.
    */
   virtual bool isOpen() const
   {
      return mOpen;
   }

   /**
    * Reconfigures the I/O device so that it is in blocking or non-blocking
    * mode depending on the given parameter.  If the device has not been
    * opened yet, it will be opened in blocking or non-blocking mode
    * appropriately when open() is called.
    *
    * @post Processes will block when accessing the device.
    *
    * @param blocking A true value puts the I/O device into blocking mode.
    *                 A false value puts it into non-blocking mode.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the device's blocking
    *         mode is set to blocking.<br>
    *         vpr::ReturnStatus::Fail is returned otherwise.
    */
   virtual ReturnStatus setBlocking(bool blocking) = 0;

   /**
    * Gets the current blocking state for the I/O device.
    *
    * @pre mBlocking is set correctly.
    *
    * @return true is returned if the device is in blocking mode.
    *         Otherwise, false is returned.
    */
   virtual bool isBlocking() const
   {
      return mBlocking;
   }

   /**
    * Gets the vpr::IOSys::Handle object for this I/O device.
    *
    * @return vpr::IOSys::NullHandle is returned if the device
    *         has no handle or if the handle could not be returned for some
    *         reason.
    */
   virtual IOSys::Handle getHandle() const = 0;

   /**
    * Reads at most the specified number of bytes from the I/O device into
    * the given buffer.
    *
    * @pre The device is open for reading, and the buffer is at least
    *      <code>length</code> bytes long.
    * @post The given buffer has <code>length</code> bytes copied into it from
    *       the device, and the number of bytes read successfully is returned
    *       to the caller via the <code>bytes_read</code> argument.
    *
    * @param buffer     A pointer to the buffer where the device's buffer
    *                   contents are to be stored.
    * @param length     The number of bytes to be read.
    * @param bytes_read The number of bytes read into the buffer.
    * @param timeout    The maximum amount of time to wait for data to be
    *                   available for reading.  This argument is optional
    *                   and defaults to vpr::Interval::NoTimeout.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the read operation
    *         completed successfully.<br>
    *         vpr::ReturnStatus::WouldBlock if the device is in non-blocking
    *         mode, and there is no data to read.<br>
    *         vpr::ReturnStatus::Timeout is returned if the read could not
    *         begin within the timeout interval.<br>
    *         vpr::ReturnStatus::Fail is returned if the read operation failed.
    */
   vpr::ReturnStatus read(void* buffer, const vpr::Uint32 length,
                          vpr::Uint32& bytes_read,
                          const vpr::Interval timeout = vpr::Interval::NoTimeout)
   {
      return this->read_s(buffer, length, bytes_read, timeout);
   }

   /**
    * Reads at most the specified number of bytes from the I/O device into
    * the given buffer.
    *
    * @pre The device is open for reading, and the buffer is at least
    *      <code>length</code> bytes long.
    * @post The given buffer has <code>length</code> bytes copied into it
    *       from the device, and the number of bytes read successfully is
    *       returned to the caller via the <code>bytes_read</code> argument.
    *
    * @param buffer     A reference to the buffer (a <code>std::string</code>
    *                   object) where the device's buffer contents are to be
    *                   stored.
    * @param length     The number of bytes to be read.
    * @param bytes_read The number of bytes read into the buffer.
    * @param timeout    The maximum amount of time to wait for data to be
    *                   available for reading.  This argument is optional
    *                   and defaults to vpr::Interval::NoTimeout.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the read operation
    *         completed successfully.<br>
    *         vpr::ReturnStatus::WouldBlock if the device is in non-blocking
    *         mode, and there is no data to read.<br>
    *         vpr::ReturnStatus::Timeout is returned if the read could not begin
    *         within the timeout interval.<br>
    *         vpr::ReturnStatus::Fail is returned if the read operation failed.
    */
   vpr::ReturnStatus read(std::string& buffer, const vpr::Uint32 length,
                          vpr::Uint32& bytes_read,
                          const vpr::Interval timeout = vpr::Interval::NoTimeout)
   {
      vpr::ReturnStatus status;

      // Allocate the temporary buffer, zero it, and read in the current
      // buffer from the device.
      buffer.resize(length);
      memset(&buffer[0], '\0', buffer.size());
      status = this->read(&buffer[0], buffer.size(), bytes_read, timeout);

      return status;
   }

   /**
    * Reads at most the specified number of bytes from the I/O device into
    * the given buffer.
    *
    * @pre The device is open for reading, and the buffer is at least
    *      <code>length</code> bytes long.
    * @post The given buffer has <code>length</code> bytes copied into it
    *       from the device, and the number of bytes read successfully is
    *       returned to the caller via the <code>bytes_read</code> argument.
    *
    * @param buffer     A pointer to the buffer (a vector of
    *                   <code>char</code>s) where the device's buffer
    *                   contents are to be stored.
    * @param length     The number of bytes to be read.
    * @param bytes_read The number of bytes read into the buffer.
    * @param timeout    The maximum amount of time to wait for data to be
    *                   available for reading.  This argument is optional
    *                   and defaults to vpr::Interval::NoTimeout.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the read operation
    *         completed successfully.<br>
    *         vpr::ReturnStatus::WouldBlock if the device is in non-blocking
    *         mode, and there is no data to read.<br>
    *         vpr::ReturnStatus::Timeout is returned if the read could not begin
    *         within the timeout interval.<br>
    *         vpr::ReturnStatus::Fail is returned if the read operation failed.
    */
   vpr::ReturnStatus read(std::vector<vpr::Uint8>& buffer,
                          const vpr::Uint32 length, vpr::Uint32& bytes_read,
                          const vpr::Interval timeout = vpr::Interval::NoTimeout)
   {
      vpr::ReturnStatus status;
      buffer.resize(length);

      // Allocate the temporary buffer, zero it, and read in the current
      // buffer from the device.
      memset(&buffer[0], '\0', buffer.size());
      status = this->read(&buffer[0], buffer.size(), bytes_read, timeout);

      // size it down if needed, if (bytes_read==length), then resize does
      // nothing...
      if ( status.success() )
      {
         buffer.resize(bytes_read);
      }

      return status;
   }

   /**
    * Reads exactly the specified number of bytes from the I/O device into
    * the given buffer.
    *
    * @pre The device is open for reading, and the buffer is at least
    *      <code>length</code> bytes long.
    * @post The given buffer has <code>length</code> bytes copied into it from
    *       the device, and the number of bytes read successfully is returned
    *       to the caller via the <code>bytes_read</code> parameter.
    *
    * @param buffer     A pointer to the buffer where the device's buffer
    *                   contents are to be stored.
    * @param length     The number of bytes to be read.
    * @param bytes_read The number of bytes read into the buffer.
    * @param timeout    The maximum amount of time to wait for data to be
    *                   available for reading.  This argument is optional
    *                   and defaults to vpr::Interval::NoTimeout.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the read operation
    *         completed successfully.<br>
    *         vpr::ReturnStatus::WouldBlock if the device is in non-blocking
    *         mode, and there is no data to read.<br>
    *         vpr::ReturnStatus::Fail is returned if the read operation failed.
    */
   vpr::ReturnStatus readn(void* buffer, const vpr::Uint32 length,
                           vpr::Uint32& bytes_read,
                           const vpr::Interval timeout = vpr::Interval::NoTimeout)
   {
      return this->readn_s(buffer, length, bytes_read, timeout);
   }

   /**
    * Reads exactly the specified number of bytes from the I/O device into
    * the given buffer.
    *
    * @pre The device is open for reading, and the buffer is at least
    *      <code>length</code> bytes long.
    * @post The given buffer has <code>length</code> bytes copied into it from
    *       the device, and the number of bytes read successfully is returned
    *       to the caller via the <code>bytes_read</code> parameter.
    *
    * @param buffer     A reference to the buffer (a <code>std::string</code>
    *                   object) where the device's buffer contents are to be
    *                   stored.
    * @param length     The number of bytes to be read.
    * @param bytes_read The number of bytes read into the buffer.
    * @param timeout    The maximum amount of time to wait for data to be
    *                   available for reading.  This argument is optional
    *                   and defaults to vpr::Interval::NoTimeout.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the read
    *         operation completed successfully.<br>
    *         vpr::ReturnStatus::WouldBlock if the device is in non-blocking
    *         mode, and there is no data to read.<br>
    *         vpr::ReturnStatus::Fail is returned if the read operation failed.
    */
   vpr::ReturnStatus readn(std::string& buffer, const vpr::Uint32 length,
                           vpr::Uint32& bytes_read,
                           const vpr::Interval timeout = vpr::Interval::NoTimeout)
   {
      vpr::ReturnStatus status;

      // Allocate the temporary buffer, zero it, and read in the current
      // buffer from the device.
      buffer.resize(length);
      memset(&buffer[0], '\0', buffer.size());
      status = this->readn(&buffer[0], buffer.size(), bytes_read, timeout);

      if ( status.success() )
      {
         buffer.resize(bytes_read);
      }

      return status;
   }

   /**
    * Reads exactly the specified number of bytes from the I/O device into
    * the given buffer.
    *
    * @pre The device is open for reading, and the buffer is at least
    *      <code>length</code> bytes long.
    * @post The given buffer has <code>length</code> bytes copied into it from
    *       the device, and the number of bytes read successfully is returned
    *       to the caller.
    *
    * @param buffer     A pointer to the buffer (a vector of
    *                   <code>char</code>s) where the device's buffer contents
    *                   are to be stored.
    * @param length     The number of bytes to be read.
    * @param bytes_read The number of bytes read into the buffer.
    * @param timeout    The maximum amount of time to wait for data to be
    *                   available for reading.  This argument is optional
    *                   and defaults to vpr::Interval::NoTimeout.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the read operation
    *         completed successfully.<br>
    *         vpr::ReturnStatus::WouldBlock if the device is in non-blocking
    *         mode, and there is no data to read.<br>
    *         vpr::ReturnStatus::Timeout is returned if the read could not begin
    *         within the timeout interval.<br>
    *         vpr::ReturnStatus::Fail is returned if the read operation failed.
    */
   vpr::ReturnStatus readn(std::vector<vpr::Uint8>& buffer,
                           const vpr::Uint32 length, vpr::Uint32& bytes_read,
                           const vpr::Interval timeout = vpr::Interval::NoTimeout)
   {
      vpr::ReturnStatus status;

      // Allocate the temporary buffer, zero it, and read in the current
      // buffer from the device.
      
      buffer.resize(length);
      memset(&buffer[0], '\0', buffer.size());
      status = this->readn(&buffer[0], buffer.size(), bytes_read, timeout);
      
      if ( status.success() )
      {
         buffer.resize(bytes_read);
      }

      return status;
   }

   /** Return the number of avaiable bytes for reading. */
   virtual vpr::Uint32 availableBytes() const
   {
      return 0;
   }

   /**
    * Write the buffer to the I/O device.
    *
    * @pre The device is open for writing.
    * @post The given buffer is written to the I/O device, and the number of
    *       bytes written successfully is returned to the caller via the
    *       <code>bytes_written</code> parameter.
    *
    * @param buffer        A pointer to the buffer to be written.
    * @param length        The length of the buffer.
    * @param bytes_written The number of bytes written to the device.
    * @param timeout       The maximum amount of time to wait for data to be
    *                      available for writing.  This argument is optional
    *                      and defaults to vpr::Interval::NoTimeout.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the write operation
    *         completed successfully.<br>
    *         vpr::ReturnStatus::WouldBlock is returned if the handle is in
    *         non-blocking mode, and the write operation could not be
    *         completed.<br>
    *         vpr::ReturnStatus::Timeout is returned if the write could not
    *         begin within the timeout interval.<br>
    *         vpr::ReturnStatus::Fail is returned if the write operation
    *         failed.
    */
   vpr::ReturnStatus write(const void* buffer, const vpr::Uint32 length,
                           vpr::Uint32& bytes_written,
                           const vpr::Interval timeout = vpr::Interval::NoTimeout)
   {
      return this->write_s(buffer, length, bytes_written,timeout);
   }

   /**
    * Writes the buffer to the I/O device.
    *
    * @pre The device is open for writing.
    * @post The given buffer is written to the I/O device, and the number of
    *       bytes written successfully is returned to the caller via the
    *       <code>bytes_written</code> parameter.
    *
    * @param buffer        A reference to the buffer (a std::string object)
    *                      to be written.
    * @param length        The length of the buffer.
    * @param bytes_written The number of bytes written to the device.
    * @param timeout       The maximum amount of time to wait for data to be
    *                      available for writing.  This argument is optional
    *                      and defaults to vpr::Interval::NoTimeout.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the write operation
    *         completed successfully.<br>
    *         vpr::ReturnStatus::WouldBlock is returned if the handle is in
    *         non-blocking mode, and the write operation could not be
    *         completed.<br>
    *         vpr::ReturnStatus::Timeout is returned if the write could not
    *         begin within the timeout interval.<br>
    *         vpr::ReturnStatus::Fail is returned if the write operation failed.
    */
   vpr::ReturnStatus write(const std::string& buffer, const vpr::Uint32 length,
                           vpr::Uint32& bytes_written,
                           const vpr::Interval timeout = vpr::Interval::NoTimeout)
   {
      vprASSERT(length <= buffer.size() && "length was bigger than the data given");
      return this->write(buffer.c_str(), length, bytes_written,timeout);
   }

   /**
    * Writes the buffer to the I/O device.
    *
    * @pre The device is open for writing.
    * @post The given buffer is written to the I/O device, and the number of
    *       bytes written successfully is returned to the caller via the
    *       <code>bytes_written</code> parameter.
    *
    * @param buffer        A pointer to the buffer (a vector of chars) to
    *                      be written.
    * @param length        The length of the buffer.
    * @param bytes_written The number of bytes written to the device.
    * @param timeout       The maximum amount of time to wait for data to be
    *                      available for writing.  This argument is optional
    *                      and defaults to vpr::Interval::NoTimeout.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the write operation
    *         completed successfully.<br>
    *         vpr::ReturnStatus::WouldBlock is returned if the handle is in
    *         non-blocking mode, and the write operation could not be
    *         completed.<br>
    *         vpr::ReturnStatus::Timeout is returned if the write could not
    *         begin within the timeout interval.<br>
    *         vpr::ReturnStatus::Fail is returned if the write operation failed.
    */
   vpr::ReturnStatus write(const std::vector<vpr::Uint8>& buffer,
                           const vpr::Uint32 length,
                           vpr::Uint32& bytes_written,
                           const vpr::Interval timeout = vpr::Interval::NoTimeout)
   {
      vprASSERT(length <= buffer.size() && "length was bigger than the data given");
      return this->write(&buffer[0], length, bytes_written,timeout);
   }

   /**
    * Tests if reading from this I/O device will block.
    *
    * @pre getHandle() returns a valid vpr::IOSys::Handle object.
    * @post
    *
    * @param timeout Give a timeout period to wait for the device to
    *                unblock for reading.  This is optional and defaults to
    *                vpr::Interval::NoWait which will simply poll the device
    *                and return immediately.
    *
    * @return <code>true</code> is returned if reading will block;
    *         <code>false</code> otherwise.
    */
   bool isReadBlocked(const vpr::Interval& timeout = vpr::Interval::NoWait)
   {
      bool is_blocked;
      vpr::Selector selector;
      vpr::IOSys::Handle handle;
      vpr::Uint16 num_events;
      vpr::ReturnStatus status;

      handle = getHandle();
      selector.addHandle(handle);
      selector.setIn(handle, vpr::Selector::Read);

      // Test the handle to get its read state.
      status = selector.select(num_events, timeout);

      if ( num_events == 1 )
      {
         is_blocked = false;
      }
      else
      {
         is_blocked = true;
      }

      return is_blocked;
   }

   /**
    * Tests if writing to this I/O device will block.
    *
    * @pre getHandle() returns a valid vpr::IOSys::Handle object.
    * @post
    *
    * @param timeout Give a timeout period to wait for the device to
    *                unblock for writing.  This is optional and defaults to
    *                vpr::Interval::NoWait which will simply poll the device
    *                and return immediately.
    *
    * @return <code>true</code> is returned if writing will block;
    *         <code>false</code> otherwise.
    */
   bool isWriteBlocked(const vpr::Interval& timeout = vpr::Interval::NoWait)
   {
      bool is_blocked;
      vpr::Selector selector;
      vpr::IOSys::Handle handle;
      vpr::Uint16 num_events;
      vpr::ReturnStatus status;

      handle = getHandle();
      selector.addHandle(handle);
      selector.setIn(handle, vpr::Selector::Write);

      // Test the handle to get its write state.
      status = selector.select(num_events, timeout);

      if ( num_events == 1 )
      {
         is_blocked = false;
      }
      else
      {
         is_blocked = true;
      }

      return is_blocked;
   }

   /**
    * Sets the IO stats strategy to use.
    */
   void setIOStatStrategy(vpr::BaseIOStatsStrategy* strat)
   {
      mStatsStrategy = strat;
      if(mStatsStrategy != NULL)              // If we have a non-NULL strategy
         mStatsStrategy->setRealObject(this); // Tell it about us
   }

   /**
    * Gets the current IO stats strategy.
    */
   vpr::BaseIOStatsStrategy* getIOStatStrategy() const
   {
      return mStatsStrategy;
   }

protected:
   /**
    * Default constructor.  This is provided for subclasses where a "named"
    * device does not make sense.
    *
    * @pre None.
    * @post The open mode is set to blocking; the open state is set to false;
    *       and the blocking mode for reads and writes is set to true.
    */
   BlockIO()
      : mOpen(false)
      , mBlocking(true)
      , mStatsStrategy(NULL)
   {
      /* Do nothing. */ ;
   }

   /**
    * Constructor.
    *
    * @pre None.
    * @post The name object is copied into mName; the open mode is set to
    *       blocking; the open state is set to <code>false</code>; and the
    *       blocking mode for reads and writes is set to <code>true</code>.
    *
    * @param name The name for this device.
    */
   BlockIO(const std::string& name)
      : mName(name)
      , mOpen(false)
      , mBlocking(true)
      , mStatsStrategy(NULL)
   {
      /* Do nothing. */ ;
   }

   /**
    * Copy constructor.
    *
    * @param other A constant reference to another vpr::BlockIO object used
    *              as the source for the copy.
    */
   BlockIO(const BlockIO& other)
      : mName(other.mName)
      , mOpen(other.mOpen)
      , mBlocking(other.mBlocking)
      , mStatsStrategy(NULL)
   {
      /* Do nothing. */ ;
   }

   /**
    * Destructor.
    *
    * @pre None.
    * @post None.
    */
   virtual ~BlockIO()
   {
      /* Do nothing. */ ;
   }

   /**
    * read strategy
    */
   virtual vpr::ReturnStatus read_s(void* buffer, const vpr::Uint32 length,
                                    vpr::Uint32& bytes_read,
                                    const vpr::Interval timeout = vpr::Interval::NoTimeout)
   {
      vpr::ReturnStatus status;

      if(mStatsStrategy != NULL)
         mStatsStrategy->read_s(status, buffer, length, bytes_read, timeout);
      else
         status = read_i(buffer, length, bytes_read, timeout);

      return status;
   }

   /**
    * read strategy
    */
   virtual vpr::ReturnStatus readn_s(void* buffer, const vpr::Uint32 length,
                                     vpr::Uint32& bytes_read,
                                     const vpr::Interval timeout = vpr::Interval::NoTimeout)
   {
      vpr::ReturnStatus status;

      if(mStatsStrategy != NULL)
         mStatsStrategy->readn_s(status, buffer, length, bytes_read, timeout);
      else
         status = readn_i(buffer, length, bytes_read, timeout);

      return status;
   }

   /**
    * write strategy
    */
   virtual vpr::ReturnStatus write_s(const void* buffer,
                                     const vpr::Uint32 length,
                                     vpr::Uint32& bytes_written,
                                     const vpr::Interval timeout = vpr::Interval::NoTimeout)
   {
      vpr::ReturnStatus status;

      if(mStatsStrategy != NULL)
         mStatsStrategy->write_s(status, buffer, length, bytes_written, timeout);
      else
          status = write_i(buffer, length, bytes_written, timeout);

      return status;
   }

   /**
    * Implementation of the read template method.  This reads at most the
    * specified number of bytes from the I/O device into the given buffer.
    *
    * @pre The device is open for reading, and the buffer is at least
    *      <code>length</code> bytes long.
    * @post The given buffer has length bytes copied into it from the device,
    *       and the number of bytes read successfully is returned to the
    *       caller via the <code>bytes_read</code> parameter.
    *
    * @param buffer     A pointer to the buffer where the device's buffer
    *                   contents are to be stored.
    * @param length     The number of bytes to be read.
    * @param bytes_read The number of bytes read into the buffer.
    * @param timeout    The maximum amount of time to wait for data to be
    *                   available for reading.  This argument is optional and
    *                   defaults to vpr::Interval::NoTimeout.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the read operation
    *         completed successfully.<br>
    *         vpr::ReturnStatus::WouldBlock if the device is in non-blocking
    *         mode, and there is no data to read.<br>
    *         vpr::ReturnStatus::Timeout is returned if the read could not
    *         begin within the timeout interval.<br>
    *         vpr::ReturnStatus::Fail is returned if the read operation failed.
    */
   virtual vpr::ReturnStatus read_i(void* buffer, const vpr::Uint32 length,
                                    vpr::Uint32& bytes_read,
                                    const vpr::Interval timeout = vpr::Interval::NoTimeout) = 0;

   /**
    * Implementation of the <code>readn</code> template method.  This reads
    * exactly the specified number of bytes from the I/O device into the
    * given buffer.
    *
    * @pre The device is open for reading, and the buffer is at least
    *      <code>length</code> bytes long.
    * @post The given buffer has <code>length</code> bytes copied into
    *       it from the device, and the number of bytes read successfully
    *       is returned to the caller via the <code>bytes_read</code>
    *       parameter.
    *
    * @param buffer     A pointer to the buffer where the device's buffer
    *                   contents are to be stored.
    * @param length     The number of bytes to be read.
    * @param bytes_read The number of bytes read into the buffer.
    * @param timeout    The maximum amount of time to wait for data to be
    *                   available for reading.  This argument is optional and
    *                   defaults to vpr::Interval::NoTimeout.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the read
    *         operation completed successfully.<br>
    *         vpr::ReturnStatus::Fail is returned if the read
    *         operation failed.<br>
    *         vpr::ReturnStatus::WouldBlock if the device is in non-blocking
    *         mode, and there is no data to read.<br>
    *         vpr::ReturnStatus::Timeout is returned if the read
    *         could not begin within the timeout interval.
    */
   virtual vpr::ReturnStatus readn_i(void* buffer, const vpr::Uint32 length,
                                     vpr::Uint32& bytes_read,
                                     const vpr::Interval timeout = vpr::Interval::NoTimeout) = 0;

   /**
    * Implementation of the <code>write</code> template method.  This writes
    * the given buffer to the I/O device.
    *
    * @pre The device is open for writing.
    * @post The given buffer is written to the I/O device, and the number
    *       of bytes written successfully is returned to the caller via the
    *       <code>bytes_written</code> parameter.
    *
    * @param buffer        A pointer to the buffer to be written.
    * @param length        The length of the buffer.
    * @param bytes_written The number of bytes written to the device.
    * @param timeout       The maximum amount of time to wait for data to be
    *                      available for writing.  This argument is optional
    *                      and defaults to vpr::Interval::NoTimeout.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the write operation
    *         completed successfully.<br>
    *         vpr::ReturnStatus::WouldBlock is returned if the handle is in
    *         non-blocking mode, and the write operation could not
    *         complete.<br>
    *         vpr::ReturnStatus::Timeout is returned if the write could not
    *         begin within the timeout interval.<br>
    *         vpr::ReturnStatus::Fail is returned if the write operation
    *         failed.
    */
   virtual vpr::ReturnStatus write_i(const void* buffer,
                                     const vpr::Uint32 length,
                                     vpr::Uint32& bytes_written,
                                     const vpr::Interval timeout = vpr::Interval::NoTimeout) = 0;

   // Friends
   friend class vpr::BaseIOStatsStrategy;    // Need it to be able to call the protected read_i, readn_i, and write_i memebers

protected:
   /// The name of the I/O device.
   std::string mName;

   /// Flag telling if the device is open.
   bool mOpen;

   /// Flag telling if blocking for reads and writes is enabled.
   bool mBlocking;

   /// Perf monitor.
   vpr::BaseIOStatsStrategy* mStatsStrategy;
};

} // End of vpr namespace


#endif   /* _VPR_BLOCK_IO_H_ */
