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
 * VR Juggler is (C) Copyright 1998-2005 by Iowa State University
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

#ifndef _VPR_FILE_HANDLE_BRIDGE_H_
#define _VPR_FILE_HANDLE_BRIDGE_H_

#include <vpr/vprConfig.h>

#include <string>
#include <vector>

#include <vpr/IO/BlockIO.h>
#include <vpr/Util/Interval.h>
#include <vpr/Util/ReturnStatus.h>


namespace vpr
{

/** \class FileHandle_t FileHandle_t.h vpr/IO/FileHandle.h
 *
 * Extension to the vpr::BlockIO interface defining a cross-platform file
 * handle interface.  Given a platform-specific wrapper class, this is
 * typedef'd to vpr::FileHandle.
 *
 * @param RealFileHandleImpl The type that serves as the true wrapper around
 *                           the platform-specific file handle implementation.
 *
 * @see vpr::FileHandleNSPR, vpr::FileHandleUNIX
 */
template<class RealFileHandleImpl>
class FileHandle_t : public BlockIO
{
public:
   /**
    * Constructor.  This initializes the member variables to reasonable
    * defaults and stores the given file name for later use.
    *
    * @pre None.
    * @post All member variables are initialized including mName that is
    *       assigned the string in file_name.
    *
    * @param fileName The name of the file to be handled.
    */
   FileHandle_t(const std::string& fileName)
      : BlockIO(file_name)
      , mHandleImpl(fileName)
   {
      /* Do nothing. */ ;
   }

   /**
    * Destructor.  This does nothing.
    *
    * @pre None.
    * @post None.
    */
   virtual ~FileHandle_t()
   {
      /* Do nothing. */ ;
   }

   /**
    * Gets the name of this file.
    *
    * @pre None.
    * @post A constant reference to the mName object is returned to the
    *       caller.
    *
    * @return An object containing the name of this file.
    */
   virtual const std::string& getName() const
   {
      return mHandleImpl.getName();
   }

   /**
    * Opens the file.
    *
    * @pre This file handle has not already been opened.
    * @post An attempt is made to open the file.  The resulting status is
    *       returned to the caller.  If opened successfully, this file is
    *       ready for use.
    *
    * @return vpr::ReturnStatus::Succeed is returned when the file was
    *         opened successfully.
    * @return vpr::ReturnStatus::Fail is returned if the file could not be
    *         opened for some reason.
    */
   virtual vpr::ReturnStatus open()
   {
      return mHandleImpl.open();
   }

   /**
    * Closes the file.  All pending operatings (as queued by the OS) are
    * completed.
    *
    * @pre This file handle is open.
    * @post An attempt is made to close the file.  The resulting status is
    *       returned to the caller.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the file was closed
    *         successfully.
    * @return vpr::ReturnStatus::Fail is returned if the file could not be
    *         closed.
    */
   virtual vpr::ReturnStatus close()
   {
      return mHandleImpl.close();
   }

   /**
    * Reconfigures the file handle so that it is in blocking or non-blocking
    * mode depending on the value of the parameter.
    *
    * @pre The file handle is open.
    * @post Processes may block when accessing the file.
    *
    * @return vpr::ReturnStatus::Succeed will be returned if the blocking mode
    *         was changed successfully.  vpr::ReturnStatus::Fail will be
    *         returned if the blocking mode could not be changed.
    */
   virtual vpr::ReturnStatus setBlocking(bool blocking)
   {
      return mHandleImpl.setBlocking(blocking);
   }

   /**
    * Gets the current blocking state for the file.
    *
    * @pre The blocking mode on the internal file handle is set correctly.
    *
    * @return true is returned when the file is in blocking mode.
    *         false is returned when the file is in non-blocking mode.
    */
   bool isBlocking() const
   {
      return mHandleImpl.isBlocking();
   }

   /**
    * Gets the vpr::IOSys::Handle object for this file.
    *
    * @return vpr::IOSys::NullHandle is returned if the file has no handle
    *         or if the handle could not be returned for some reason.
    */
   virtual IOSys::Handle getHandle() const
   {
      return mHandleImpl.getHandle();
   }

   // ========================================================================
   // vpr::FileHandle API extensions.
   // ========================================================================

   /**
    * Sets the open flags so that the file handle is opened in read-only mode.
    *
    * @pre None.
    * @post The open flags are updated so that when the device is opened, it
    *       it is opened in read-only mode.  If the device is already open,
    *       this has no effect.
    */
   void setOpenReadOnly()
   {
      mHandleImpl.setOpenReadOnly();
   }

   /**
    * Sets the open flags so that the file handle is opened in write-only
    * mode.
    *
    * @pre None.
    * @post The open flags are updated so that when the device is opened, it
    *       is opened in write-only mode.  If the device is already open,
    *       this has no effect.
    */
   void setOpenWriteOnly()
   {
      mHandleImpl.setOpenWriteOnly();
   }

   /**
    * Sets the open flags so that the file handle is opened in read/write
    * mode.
    *
    * @pre None.
    * @post The open flags are updated so that when the device is opened, it
    *       is opened in read/write mode.  If the device is already open,
    *       this has no effect.
    */
   void setOpenReadWrite()
   {
      mHandleImpl.setOpenReadWrite();
   }

   /**
    * Reconfigures the file handle to be in append mode or not.
    *
    * @pre The file handle is open.
    * @post The file handle's write mode is set to append (or not) depending
    *       on the given parameter.
    *
    * @param flag A value of true indicates that the file should be in append
    *             mode.  A value of false indicates that it should not.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the write mode was
    *         changed successfully.  vpr::ReturnStatus::Fail is returned
    *         otherwise.
    */
   vpr::ReturnStatus setAppend(bool flag)
   {
      return mHandleImpl.setAppend(flag);
   }

   /**
    * Reconfigures the file handle so that writes are synchronous or
    * asynchronous depending on the value of the given parameter.
    *
    * @pre The file handle is open.
    * @post Writes are performed synchronously.
    *
    * @param flag A value of true indicates that the file should use
    *             synchronous writes.  A value of false indicates that it
    *             should use asynchronous writes.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the write mode was
    *         changed successfully.  vpr::ReturnStatus::Fail is returned if
    *         the write mode could not be changed for some reason.
    */
   vpr::ReturnStatus setSynchronousWrite(bool flag)
   {
      return mHandleImpl.setSynchronousWrite(flag);
   }

   /**
    * Tests if the file handle is read-only.
    *
    * @pre The file handle is open.
    * @post The access mode is tested for read-only mode, and the result is
    *       returned to the caller.
    *
    * @return \c true is returned if the device is in read-only mode;
    *         \c false otherwise.
    */
   bool isReadOnly() const
   {
      return mHandleImpl.isReadOnly();
   }

   /**
    * Tests if the file handle is write-only.
    *
    * @pre The file handle is open.
    * @post The access mode is tested for write-only mode, and the result is
    *       returned to the caller.
    *
    * @return \c true is returned if the device is in write-only mode;
    *         \c false otherwise.
    */
   bool isWriteOnly() const
   {
      return mHandleImpl.isWriteOnly();
   }

   /**
    * Tests if the file handle is read/write.
    *
    * @pre The file handle is open.
    * @post The access mode is tested for read/write mode, and the result is
    *       returned to the caller.
    *
    * @return \c true is returned if the device is in read/write mode;
    *         \c false otherwise.
    */
   bool isReadWrite() const
   {
      return mHandleImpl.isReadWrite();;
   }

protected:
   /**
    * Reads at most the specified number of bytes from the file handle into
    * the given buffer.
    *
    * @pre The file handle implementation object is valid, and the buffer is
    *      at least length bytes long.
    * @post The given buffer has length bytes copied into it from the file
    *       bufffer, and the number of bytes read successfully is returned
    *       to the caller.
    *
    * @param buffer    A pointer to the buffer where the file's buffer
    *                  contents are to be stored.
    * @param length    The number of bytes to be read.
    * @param bytesRead A reference to a variable where the number of bytes
    *                  successfully read from the file will be stored.
    *                  The value will be -1 if an error occurred.
    * @param timeout   The amount of time to wait before returning to the
    *                  caller.  This argument is optional and defaults to
    *                  vpr::Interval::NoTimeout.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the read operation
    *         completed successfully.
    * @return vpr::ReturnStatus::Fail is returned if the read operation
    *         failed.
    * @return vpr::ReturnStatus::WouldBlock is returned if the handle is in
    *         non-blocking mode, and there is no data to read.
    */
   vpr::ReturnStatus read_i(void* buffer, const vpr::Uint32 length,
                            vpr::Uint32& bytesRead,
                            const vpr::Interval timeout = vpr::Interval::NoTimeout)
   {
      return mHandleImpl.read_i(buffer, length, bytesRead, timeout);
   }

   /**
    * Reads exactly the specified number of bytes from the file into the
    * given buffer.
    *
    * @pre The handle implementation object is valid, and the buffer is at
    *      least length bytes long.
    * @post The given buffer has length bytes copied into it from the file
    *       buffer, and the number of bytes read successfully is returned to
    *       the caller.
    *
    * @param buffer    A pointer to the buffer where the file's buffer
    *                  contents are to be stored.
    * @param length    The number of bytes to be read.
    * @param bytesRead A reference to a variable where the number of bytes
    *                  successfully read from the file will be stored.
    *                  The value will be -1 if an error occurred.
    * @param timeout   The amount of time to wait before returning to the
    *                  caller.  This argument is optional and defaults to
    *                  vpr::Interval::NoTimeout.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the read operation
    *         completed successfully.
    * @return vpr::ReturnStatus::Fail is returned if the read operation failed.
    */
   vpr::ReturnStatus readn_i(void* buffer, const vpr::Uint32 length,
                             vpr::Uint32& bytesRead,
                             const vpr::Interval timeout = vpr::Interval::NoTimeout)
   {
      return mHandleImpl.readn_i(buffer, length, bytesRead, timeout);
   }

   /**
    * Writes the given buffer to the file.
    *
    * @pre The handle implementation object is valid.
    * @post The given buffer is written to the file, and the number of
    *       bytes written successfully is returned to the caller.
    *
    * @param buffer       A pointer to the buffer to be written.
    * @param length       The length of the buffer.
    * @param bytesWritten A reference to a variable where the number of
    *                     bytes successfully written to the file will be
    *                     stored.  The value will be -1 if an error
    *                     occurred.
    * @param timeout      The amount of time to wait before returning to the
    *                     caller.  This argument is optional and defaults to
    *                     vpr::Interval::NoTimeout.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the write operation
    *         completed successfully.
    * @return vpr::ReturnStatus::Fail is returned if the write operation
    *         failed.
    * @return vpr::ReturnStatus::WouldBlock is returned if the handle is in
    *         non-blocking mode, and the write operation could not be
    *         completed.
    */
   vpr::ReturnStatus write_i(const void* buffer, const vpr::Uint32 length,
                             vpr::Uint32& bytesWritten,
                             const vpr::Interval timeout = vpr::Interval::NoTimeout)
   {
      return mHandleImpl.write_i(buffer, length, bytesWritten, timeout);
   }

   /// Platform-specific file hanlde implementation instance.
   RealFileHandleImpl mHandleImpl;
};

} // End of vpr namespace


#endif  /* _VPR_FILE_HANDLE_H_ */
