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

#include <vpr/vprConfig.h>

#include <iostream>
#include <stdlib.h>
#include <string.h>

#include <vpr/md/WIN32/IO/Port/SerialPortImplWin32.h>
#include <vpr/Util/Debug.h>


namespace vpr
{

// ----------------------------------------------------------------------------
// Constructor.  This sets all the default values for the given port name.
// ----------------------------------------------------------------------------
SerialPortImplWin32::SerialPortImplWin32 (const std::string& port_name)
: mName(port_name), mHandle(NULL)
{
   mOpenFlag = GENERIC_READ | GENERIC_WRITE;
   mBlocking = 0;
   mParityMark = false;
   mCurrentTimeout=0;
}

// ----------------------------------------------------------------------------
// Destructor.  If the file handle is non-NULL, its memory is released.
// ----------------------------------------------------------------------------
SerialPortImplWin32::~SerialPortImplWin32 ()
{
   if ( mOpen )
   {
      close();
   }
}

// ----------------------------------------------------------------------------
// Open the serial port and initialize its flags.
// ----------------------------------------------------------------------------
vpr::ReturnStatus SerialPortImplWin32::open()
{
   vpr::ReturnStatus status;
   COMMTIMEOUTS gct;
   mHandle = CreateFile( mName.c_str(),
                         mOpenFlag,
                         0,    // comm devices must be opened w/exclusive-access
                         NULL, // no security attributes
                         OPEN_EXISTING, // comm devices must use OPEN_EXISTING
                         mBlocking,    // not overlapped I/O
                         NULL  // hTemplate must be NULL for comm devices
                       );

   if ( mHandle == INVALID_HANDLE_VALUE )
   {  // Handle the error.
      std::cout << "CreateFile failed with error: " << GetLastError() << std::endl;
      status.setCode(vpr::ReturnStatus::Fail);
	  return status;
   }
   gct.ReadIntervalTimeout =0;
   gct.ReadTotalTimeoutConstant=0;
   gct.ReadTotalTimeoutMultiplier=0;
   gct.WriteTotalTimeoutConstant=5000;
   gct.WriteTotalTimeoutMultiplier=0;
   if ( !SetCommTimeouts(mHandle,&gct) )
   {
      std::cout << "Timeout initialization failed." << std::endl;
      status.setCode(vpr::ReturnStatus::Fail);
   }
   DCB dcb;
   GetCommState(mHandle, &dcb);
   SetCommState(mHandle, &dcb);

   setHardwareFlowControl(false);

   mOpen = true;
   return status;
}

vpr::ReturnStatus SerialPortImplWin32::close()
{
   vpr::ReturnStatus retval;

   if ( !CloseHandle(mHandle) )
   {
      retval.setCode(vpr::ReturnStatus::Fail);
   }

   mHandle=NULL;

   return retval;
}

vpr::ReturnStatus SerialPortImplWin32::setBlocking(bool blocking)
{
   vpr::ReturnStatus status;

   if ( ! mOpen )
   {
       vprDEBUG(vprDBG_ERROR, vprDBG_CRITICAL_LVL)
          << "ERROR: Enabling blocking mode after port open is unsuported in Win32."
          << std::endl << vprDEBUG_FLUSH;
   }
   else
   {
      if ( blocking )
      {
         mBlocking &= ~FILE_FLAG_OVERLAPPED;
      }
      else
      {
         // XXX: Still not sure why this is not supported.
         vprDEBUG(vprDBG_ERROR, vprDBG_CRITICAL_LVL)
            << "ERROR: Non-Blocking not currently supported in win32."
            << std::endl << vprDEBUG_FLUSH;
         status.setCode(vpr::ReturnStatus::Fail);
//         mBlocking |= FILE_FLAG_OVERLAPPED;
      }
   }

   return status;
}

vpr::SerialTypes::UpdateActionOption SerialPortImplWin32::getUpdateAction() const
{
   std::cout << "Update Action is always NOW in Win32" << std::endl;
   vpr::SerialTypes::UpdateActionOption update;
   update = SerialTypes::NOW;
   return update;
}

vpr::ReturnStatus SerialPortImplWin32::clearAll ()
{
   // Not implemented yet...
   return vpr::ReturnStatus(vpr::ReturnStatus::Fail);
}


void SerialPortImplWin32::setUpdateAction (SerialTypes::UpdateActionOption action)
{
   /* Do Nothing */
   std::cout << "Update action always NOW in Win32" << std::endl;
}

// Query the serial port for the maximum buffer size.
vpr::ReturnStatus SerialPortImplWin32::getMinInputSize(vpr::Uint16 &size) const
{
   vpr::ReturnStatus s;
   COMMPROP lpCommProp;
   if ( !GetCommProperties(mHandle, &lpCommProp) || (int)lpCommProp.dwCurrentTxQueue == 0 )
   {
      s.setCode(vpr::ReturnStatus::Fail);
      std::cout << "Maximum buffer size is unavailable.\n";
   }
   else
   {
      size = lpCommProp.dwCurrentTxQueue;
   }

   return s;
}

// Attempt to change the buffer size to the given argument.
vpr::ReturnStatus SerialPortImplWin32::setMinInputSize(const vpr::Uint8 size)
{
   ReturnStatus s;
   if ( !SetupComm(mHandle, (int)size, (int)size) )
   {
      s.setCode(vpr::ReturnStatus::Fail);
      std::cout << "Could not set the minimum buffer size - "<<GetLastError()<<"\n";
   }
   return s;
}

// Get the value of the timeout (in tenths of a second) to wait for data to
// arrive.  This is only applicable in non-canonical mode.
vpr::ReturnStatus SerialPortImplWin32::getTimeout(vpr::Uint8& timeout) const
{
   COMMTIMEOUTS t;
   vpr::ReturnStatus retval;
   if ( !GetCommTimeouts(mHandle, &t) )
   {
      retval.setCode(vpr::ReturnStatus::Fail);
      std::cout << "The timeout value is unavailable.\n";
   }
   timeout = (int)t.ReadTotalTimeoutConstant/100;
   return retval;
}

// Set the value of the timeout to wait for data to arrive.  The value given
// must be in tenths of a second.  This is only applicable in non-canonical
// mode.
vpr::ReturnStatus SerialPortImplWin32::setTimeout(const vpr::Uint8 timeout)
{
   COMMTIMEOUTS t;
   vpr::ReturnStatus retval;
   GetCommTimeouts(mHandle, &t);

   mCurrentTimeout=timeout;

   t.ReadTotalTimeoutConstant = (int)timeout*100;
   if ( !SetCommTimeouts(mHandle, &t) )
   {
      retval.setCode(vpr::ReturnStatus::Fail);
      std::cout << "Could not set timeout value.\n";
   }

   return retval;
}


// Get the character size (the bits per byte).
vpr::ReturnStatus SerialPortImplWin32::getCharacterSize(vpr::SerialTypes::CharacterSizeOption& size) const
{
   vpr::ReturnStatus retval;
   DCB dcb;
   if ( GetCommState(mHandle, &dcb) )
   {
      switch ( dcb.ByteSize )
      {
         case 5:
            size = vpr::SerialTypes::CS_BITS_5;
            break;
         case 6:
            size = vpr::SerialTypes::CS_BITS_6;
            break;
         case 7:
            size = vpr::SerialTypes::CS_BITS_7;
            break;
         case 8:
            size = vpr::SerialTypes::CS_BITS_8;
            break;
      }
   }
   else
   {
      retval.setCode(vpr::ReturnStatus::Fail);
      std::cout << "Error attaining bits/byte.\n";
   }
   return retval;
}

// Set the current character size (the bits per byte) to the size in the given
// value.  This is used for both reding and writing, and the size does not
// include the parity bit (if any).
vpr::ReturnStatus SerialPortImplWin32::setCharacterSize(const vpr::SerialTypes::CharacterSizeOption bpb)
{
   vpr::ReturnStatus retval;
   DCB dcb;
   GetCommState(mHandle, &dcb);
   switch ( bpb )
   {
      case vpr::SerialTypes::CS_BITS_5:
         dcb.ByteSize = 5;
         break;
      case vpr::SerialTypes::CS_BITS_6:
         dcb.ByteSize = 6;
         break;
      case vpr::SerialTypes::CS_BITS_7:
         dcb.ByteSize = 7;
         break;
      case vpr::SerialTypes::CS_BITS_8:
         dcb.ByteSize = 8;
         break;
   }
   if ( !SetCommState(mHandle,&dcb) )
   {
      std::cout << GetLastError()<< std::endl<<std::endl;
      retval.setCode(vpr::ReturnStatus::Fail);
      std::cout << "Error setting bits/byte.\n";
   }
   return retval;
}

vpr::ReturnStatus SerialPortImplWin32::getStopBits(vpr::Uint8& num_bits) const
{
   DCB dcb;

   vpr::ReturnStatus retval;

   if ( GetCommState(mHandle, &dcb) )
   {
      switch ( dcb.StopBits )
      {
         case ONESTOPBIT:
            num_bits = 1;
            break;
         case TWOSTOPBITS:
            num_bits = 2;
            break;
      }
   }
   else
   {
      std::cout << "Number of stop bits is unavailable.\n";
      retval.setCode(vpr::ReturnStatus::Fail);
   }
   return retval;
}


// Set the number of stop bits to use.  The value must be either 1 or 2.
vpr::ReturnStatus SerialPortImplWin32::setStopBits(const vpr::Uint8 num_bits)
{
   DCB dcb;

   vpr::ReturnStatus retval;

   GetCommState(mHandle, &dcb);
   switch ( (int)num_bits )
   {
      case 1:
         dcb.StopBits = ONESTOPBIT;
         break;
      case 2:
         dcb.StopBits = TWOSTOPBITS;
         break;
   }
   if ( !SetCommState(mHandle, &dcb) )
   {
      std::cout << "Error in setting stop bits.\n";
      retval.setCode(vpr::ReturnStatus::Fail);
   }
   return retval;
}

// Get the state of parity checking for input.
bool SerialPortImplWin32::getInputParityCheckState() const
{
   DCB dcb;
   bool b;
   if ( !GetCommState(mHandle, &dcb) )
   {
      std::cout << "Error attaining parity checking state.\n";
   }
   if ( dcb.fParity == true )
   {
      b = true;
   }
   else
   {
      b = false;
   }
   return b;
}

// Enable input parity checking.
vpr::ReturnStatus SerialPortImplWin32::setInputParityCheck(bool flag)
{
   vpr::ReturnStatus s;
   DCB dcb;

   if ( !GetCommState(mHandle, &dcb) )
   {
      s.setCode(vpr::ReturnStatus::Fail);
   }

   dcb.fParity = flag;
   if ( !SetCommState(mHandle, &dcb) )
   {
      s.setCode(vpr::ReturnStatus::Fail);
   }

   return s;
}

// Get the current CLOCAL state, if the device is locally attached.
bool SerialPortImplWin32::getLocalAttachState() const
{
   // Not implemented yet...
   return false;
}

// Enable CLOCAL, that is the device is locally attached
vpr::ReturnStatus SerialPortImplWin32::setLocalAttach(bool flag)
{
   // Not implemented yet...
   return vpr::ReturnStatus(vpr::ReturnStatus::Fail);
}

// Get the current state of ignoring BREAK bytes.
bool SerialPortImplWin32::getBreakByteIgnoreState() const
{
   // Not implemented yet...
   return false;
}

// Enable ignoring of received BREAK bytes.
vpr::ReturnStatus SerialPortImplWin32::setBreakByteIgnore(bool flag)
{
   // Not implemented yet...
   return vpr::ReturnStatus(vpr::ReturnStatus::Fail);
}

// Get the current parity checking type (either odd or even).
vpr::SerialTypes::ParityType SerialPortImplWin32::getParity() const
{
   DCB dcb;
   GetCommState(mHandle, &dcb);
   if ( dcb.Parity == EVENPARITY )
   {
      return vpr::SerialTypes::PORT_PARITY_EVEN;
   }
   if ( dcb.Parity == ODDPARITY )
   {
      return vpr::SerialTypes::PORT_PARITY_ODD;
   }
   else
   {
      std::cout << "error in attaining parity type\n";
      return vpr::SerialTypes::PORT_PARITY_ODD;
   }
}

// Enable odd or even parity.
vpr::ReturnStatus SerialPortImplWin32::setParity(const vpr::SerialTypes::ParityType& type)
{
   ReturnStatus s;
   DCB dcb;
   GetCommState(mHandle, &dcb);

   if ( type == vpr::SerialTypes::PORT_PARITY_ODD )
   {
      dcb.Parity = ODDPARITY;
   }
   else
   {
      dcb.Parity = EVENPARITY;
   }

   SetCommState(mHandle, &dcb);
   return s;
}

vpr::ReturnStatus SerialPortImplWin32::write_i(const void* buffer,
                                               const vpr::Uint32 length,
                                               vpr::Uint32& bytes_written,
                                               const vpr::Interval timeout)
{
   vpr::ReturnStatus s;
   unsigned long bytes;

   if ( vpr::Interval::NoTimeout != timeout )
      vprDEBUG(vprDBG_ALL,vprDBG_WARNING_LVL) << "Timeout not supported\n" << vprDEBUG_FLUSH;

   if ( !WriteFile(mHandle, buffer, length, &bytes, NULL) )
   {
      s.setCode(vpr::ReturnStatus::Fail);
      bytes_written = 0;
   }else{
      bytes_written = bytes;
   }
   return s;


}

vpr::ReturnStatus SerialPortImplWin32::read_i(void* buffer, const vpr::Uint32 length,
                                              vpr::Uint32& bytes_read,
                                              const vpr::Interval timeout)
{
   vpr::ReturnStatus s;
   unsigned long bytes;

   //Shouldn't be setting this every read, but don't have any other way of specifying the timeout
   if ( vpr::Interval::NoTimeout != timeout )
   {
      COMMTIMEOUTS t;
      GetCommTimeouts(mHandle, &t);
      t.ReadTotalTimeoutConstant = (int)timeout.msec();
      SetCommTimeouts(mHandle, &t);
   }

   if ( !ReadFile( mHandle, buffer, length, &bytes,NULL) )
   {
      s.setCode(vpr::ReturnStatus::Fail);
   }

   bytes_read = bytes;

   if(bytes==0){
      s.setCode(vpr::ReturnStatus::Timeout);
   }

   //Now set the timeout back
   if ( vpr::Interval::NoTimeout != timeout )
   {
      COMMTIMEOUTS t;
      GetCommTimeouts(mHandle, &t);
      t.ReadTotalTimeoutConstant = (int)mCurrentTimeout*100;
      SetCommTimeouts(mHandle, &t);
   }

   return s;
}

vpr::ReturnStatus SerialPortImplWin32::readn_i(void* buffer, const vpr::Uint32 length,
                                              vpr::Uint32& bytes_read,
                                              const vpr::Interval timeout)
{
	//Call read_i for now
	return read_i(buffer,length,bytes_read,timeout);
}

// Get the current state of ignoring bytes with framing errors (other than a
// BREAK) or parity errors.
bool SerialPortImplWin32::getBadByteIgnoreState() const
{
   DCB dcb;
   GetCommState(mHandle, &dcb);
   return dcb.fErrorChar;
}

// Enable ignoring of received bytes with framing errors or parity errors.
vpr::ReturnStatus SerialPortImplWin32::setBadByteIgnore(bool flag)
{
   vpr::ReturnStatus s;
   DCB dcb;
   GetCommState(mHandle, &dcb);
   dcb.fErrorChar = flag;

   if ( !SetCommState(mHandle, &dcb) )
   {
      s.setCode(vpr::ReturnStatus::Fail);
   }
   return s;
}

// Get the current state of parity generation for outgoing bytes and parity
// checking for incoming bytes.
bool SerialPortImplWin32::getParityGenerationState() const
{
   DCB dcb;
   GetCommState(mHandle, &dcb);
   if ( dcb.fParity = false )
   {
      std::cout << "parity checking is not true\n";
      return false;
   }
   else if ( dcb.Parity != NOPARITY )
   {
      std::cout << "parity generaton not invoked\n";
      return false;
   }
   else
   {
      return true;
   }

}

// Enable parity generation for outgoing bytes and parity checking for
// incoming bytes.
vpr::ReturnStatus SerialPortImplWin32::setParityGeneration(bool enableParity)
{
   vpr::ReturnStatus s;
   DCB dcb;
   GetCommState(mHandle, &dcb);
   dcb.fParity = enableParity;

   if ( enableParity )
   {
      vpr::SerialTypes::ParityType p = getParity();
      if ( dcb.Parity == NOPARITY )
      {
         dcb.Parity = ODDPARITY;
      }
      else
      {
         dcb.Parity = p;
      }
   }
   else
   {
      dcb.Parity = NOPARITY;
   }

   if ( !SetCommState(mHandle, &dcb) )
   {
      s.setCode(vpr::ReturnStatus::Fail);
   }
   return s;
}

// Enable marking of bytes with parity errors or framing errors (except
// BREAKs).  This is only active if input parity and framing error reporting
// is enabled (see getInputParityCheckState() for more information).  The mark
// is the three-byte sequence \377 \0 X where X is the byte received in error.
// If bit stripping is enabled, a valid \377 byte is passed as the two-byte
// sequence \377 \377.
bool SerialPortImplWin32::getParityErrorMarkingState() const
{
   return mParityMark;
}

// Enable parity error and framing error marking.
vpr::ReturnStatus SerialPortImplWin32::setParityErrorMarking(bool flag)
{
   vpr::ReturnStatus s;
   DCB dcb;
   GetCommState(mHandle, &dcb);
   dcb.fErrorChar = flag;
   if ( !SetCommState(mHandle, &dcb) )
   {
      s.setCode(vpr::ReturnStatus::Fail);
   }

   return s;
}

// Get the current input baud rate.
vpr::ReturnStatus SerialPortImplWin32::getInputBaudRate(vpr::Uint32& rate) const
{
   vpr::ReturnStatus s;
   DCB dcb;
   GetCommState(mHandle, &dcb);
   rate = dcb.BaudRate;
   return s;
}

// Set the current input baud rate.
vpr::ReturnStatus SerialPortImplWin32::setInputBaudRate(const vpr::Uint32& baud)
{
   vpr::ReturnStatus s;
   DCB dcb;
   GetCommState(mHandle, &dcb);
   dcb.BaudRate = baud;
   if ( !SetCommState(mHandle, &dcb) )
   {
      s.setCode(vpr::ReturnStatus::Fail);
   }
   return s;

}

// Get the current output baud rate.
vpr::ReturnStatus SerialPortImplWin32::getOutputBaudRate(vpr::Uint32& rate) const
{
   vpr::ReturnStatus s;
   DCB dcb;
   GetCommState(mHandle, &dcb);
   rate = dcb.BaudRate;
   return s;
}

// Set the current output baud rate.
vpr::ReturnStatus SerialPortImplWin32::setOutputBaudRate(const vpr::Uint32& baud)
{
   vpr::ReturnStatus s;
   DCB dcb;
   GetCommState(mHandle, &dcb);
   dcb.BaudRate =baud;
   if ( !SetCommState(mHandle, &dcb) )
   {
      s.setCode(vpr::ReturnStatus::Fail);
   }
   return s;
}

// Transmit a continuous stream of zero bits for the given duration.  If the
// argument is 0, the transmission will last between 0.25 and 0.5 seconds.
// Otherwise, the duration specfies the number of seconds to send the zero bit
// stream.
vpr::ReturnStatus SerialPortImplWin32::sendBreak(const vpr::Int32 duration)
{
   vpr::ReturnStatus s;
   DWORD flags;

   //Send a break for .5 seconds
   SetCommBreak(mHandle); 
   Sleep(500);
   ClearCommBreak(mHandle);
   Sleep(35);
   ClearCommError(mHandle,&flags,NULL);	//Clear the break error

   return s;
}

vpr::ReturnStatus SerialPortImplWin32::drainOutput()
{
   vpr::ReturnStatus s;
   // do nothing
   return s;
}

vpr::ReturnStatus SerialPortImplWin32::controlFlow(SerialTypes::FlowActionOption opt)
{
   vpr::ReturnStatus s;
   // do nothing
   return s;
}

bool SerialPortImplWin32::getHardwareFlowControlState() const
{
   DCB dcb;
   GetCommState(mHandle, &dcb);
   if(dcb.fRtsControl==RTS_CONTROL_DISABLE && dcb.fDtrControl==DTR_CONTROL_DISABLE)
   {
      return false;
   }
   else
   {
      return true;
   }
}

vpr::ReturnStatus SerialPortImplWin32::setHardwareFlowControl(bool enable)
{
   vpr::ReturnStatus s;

   DCB dcb;
   GetCommState(mHandle, &dcb);

   if ( enable )
   {
      dcb.fRtsControl=RTS_CONTROL_ENABLE;
      dcb.fDtrControl=DTR_CONTROL_ENABLE;
   }
   else
   {
      dcb.fRtsControl=RTS_CONTROL_DISABLE;
      dcb.fDtrControl=DTR_CONTROL_DISABLE;
   }
   SetCommState(mHandle,&dcb);

   return s;
}

vpr::ReturnStatus SerialPortImplWin32::flushQueue(vpr::SerialTypes::FlushQueueOption queue)
{
   vpr::ReturnStatus s;

   if(queue==vpr::SerialTypes::INPUT_QUEUE || queue==vpr::SerialTypes::IO_QUEUES)
   {
      PurgeComm(mHandle, PURGE_RXCLEAR);
   }

   if(queue==vpr::SerialTypes::OUTPUT_QUEUE || queue==vpr::SerialTypes::IO_QUEUES)
   {
      PurgeComm(mHandle, PURGE_TXCLEAR);
   }

   return s;
}


bool SerialPortImplWin32::getCanonicalState() const
{
   std::cout << "Canonical State not yet implemented, EOF is enabled."
             << std::endl;
   return false;
}

vpr::ReturnStatus SerialPortImplWin32::setCanonicalInput(bool flag)
{
   std::cout << "Canoncial State not yet implemented, EOF is enabled."
             << std::endl;
   return vpr::ReturnStatus::Fail;
}

bool SerialPortImplWin32::getBitStripState() const
{
   std::cout << "Bit Stripping is not yet implemented on Win32." << std::endl;
   return false;
}

vpr::ReturnStatus SerialPortImplWin32::setBitStripping(bool flag)
{
   std::cout << "Bit Stripping is not yet implemented on Win32." << std::endl;
   return vpr::ReturnStatus::Fail;
}

bool SerialPortImplWin32::getStartStopInputState() const
{
   std::cout << "Start/Stop Input is not yet implemented on Win32."
             << std::endl;
   return false;
}

bool SerialPortImplWin32::getStartStopOutputState() const
{
   std::cout << "Start/Stop Output is not yet implemented on Win32."
             << std::endl;
   return false;
}

vpr::ReturnStatus SerialPortImplWin32::setStartStopInput(bool flag)
{
   std::cout << "Start/Stop Input is not yet implemented on Win32."
             << std::endl;
   return vpr::ReturnStatus::Fail;
}

vpr::ReturnStatus SerialPortImplWin32::setStartStopOutput(bool flag)
{
   std::cout << "Start/Stop output is not yet implemented on Win32."
             << std::endl;
   return vpr::ReturnStatus::Fail;
}

} // End of vpr namespace
