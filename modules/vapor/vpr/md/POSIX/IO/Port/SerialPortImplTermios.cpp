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

#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <errno.h>

#include <vpr/Util/Debug.h>
#include <vpr/md/POSIX/IO/Port/SerialPortImplTermios.h>


namespace vpr
{

// ============================================================================
// Public methods.
// ============================================================================

// Constructor.  This creates a file handle object connected to the given port
// name and sets the update action to happen immediately.
SerialPortImplTermios::SerialPortImplTermios(const std::string& port_name)
{
   mHandle = new FileHandleImplUNIX(port_name);
   setUpdateAction(SerialTypes::NOW);
}

// Destructor.  If the file handle is non-NULL, its memory is released.
SerialPortImplTermios::~SerialPortImplTermios()
{
   if ( mHandle != NULL )
   {
      delete mHandle;
   }
}

// Open the serial port and initialize its flags.
vpr::ReturnStatus SerialPortImplTermios::open()
{
   vprASSERT(mHandle->mFdesc == -1 && "The port may already be open");
   vpr::ReturnStatus status;

   status = mHandle->open();

   // If the serial port could not be opened, print an error message.
   if ( status == vpr::ReturnStatus::Fail )
   {
      vprDEBUG(vprDBG_ERROR, vprDBG_CRITICAL_LVL)
         << clrOutBOLD(clrRED, "ERROR:")
         << " [vpr::SerialPortImplTermios::open()] Could not open serial port "
         << getName() << std::endl << vprDEBUG_FLUSH;
      vprDEBUG_NEXT(vprDBG_ERROR, vprDBG_CRITICAL_LVL)
         << strerror(errno) << std::endl << vprDEBUG_FLUSH;
   }

   return status;
}

// Get the current update action.  This tells when updates to the serial
// device take effect.
vpr::SerialTypes::UpdateActionOption SerialPortImplTermios::getUpdateAction() const
{
   vpr::SerialTypes::UpdateActionOption action;

   switch ( mActions )
   {
      case TCSANOW:
         action = SerialTypes::NOW;
         break;
      case TCSADRAIN:
         action = SerialTypes::DRAIN;
         break;
      case TCSAFLUSH:
         action = SerialTypes::FLUSH;
         break;
      default:
         action = SerialTypes::NOW;
         break;
   }

   return action;
}

// Clear all flags by setting them to 0. This is mainly needed by Linux
// because IRIX does this automatically.
vpr::ReturnStatus SerialPortImplTermios::clearAll()
{
    struct termios term;
    vpr::ReturnStatus retval;
    if ( (retval = getAttrs(&term)).success() )
    {
        std::string msg;
        term.c_cflag = 0;
        term.c_lflag = 0;
        term.c_iflag = 0;
        term.c_oflag = 0;
        term.c_cc[ VMIN ] = 1;   //Set default to 1, setting 0 would be dangerous

        // Construct the error message to send to setAttrs().
        msg = "Could set Clear All settings";

        retval = setAttrs(&term, msg);
    }
    return retval;
}

// Change the current update action to take place as described by the given
// value.
void SerialPortImplTermios::setUpdateAction(vpr::SerialTypes::UpdateActionOption action)
{
   switch ( action )
   {
      case SerialTypes::NOW:
         mActions = TCSANOW;
         break;
      case SerialTypes::DRAIN:
         mActions = TCSADRAIN;
         break;
      case SerialTypes::FLUSH:
         mActions = TCSAFLUSH;
         break;
   }
}

// Query the serial port for the maximum buffer size.
vpr::ReturnStatus SerialPortImplTermios::getMinInputSize(vpr::Uint16& size) const
{
   vprASSERT(mHandle->mFdesc != -1 && "The port may not be open");
   vpr::ReturnStatus retval;
   struct termios term;

   if ( (retval = getAttrs(&term)).success() )
   {
      size = (vpr::Uint16) term.c_cc[VMIN];
   }

   return retval;
}

// Attempt to change the buffer size to the given argument.
vpr::ReturnStatus SerialPortImplTermios::setMinInputSize(const vpr::Uint8 size)
{
   vpr::ReturnStatus retval;
   struct termios term;

   if ( (retval = getAttrs(&term)).success() )
   {
      term.c_cc[VMIN] = size;
      retval = setAttrs(&term, "Could not set minimum buffer size");
   }

   return retval;
}

// Get the value of the timeout (in tenths of a second) to wait for data to
// arrive.  This is only applicable in non-canonical mode.
vpr::ReturnStatus SerialPortImplTermios::getTimeout(vpr::Uint8& timeout) const
{
   vpr::ReturnStatus retval;
   struct termios term;

   if ( (retval = getAttrs(&term)).success() )
   {
      timeout = term.c_cc[VTIME];
   }

   return retval;
}

// Set the value of the timeout to wait for data to arrive.  The value given
// must be in tenths of a second.  This is only applicable in non-canonical
// mode.
vpr::ReturnStatus SerialPortImplTermios::setTimeout(const vpr::Uint8 timeout)
{
   vpr::ReturnStatus retval;
   struct termios term;

   if ( (retval = getAttrs(&term)).success() )
   {
      term.c_cc[VTIME] = timeout;
      retval = setAttrs(&term, "Could not set minimum buffer size");
   }

   return retval;
}

// Get the character size (the bits per byte).
vpr::ReturnStatus SerialPortImplTermios::getCharacterSize(vpr::SerialTypes::CharacterSizeOption& size) const
{
   vpr::ReturnStatus retval;
   struct termios term;

   if ( (retval = getAttrs(&term)).success() )
   {
      switch ( term.c_cflag & CSIZE )
      {
         case CS5:
            size = SerialTypes::CS_BITS_5;
            break;
         case CS6:
            size = SerialTypes::CS_BITS_6;
            break;
         case CS7:
            size = SerialTypes::CS_BITS_7;
            break;
         case CS8:
            size = SerialTypes::CS_BITS_8;
            break;
      }
   }

   return retval;
}

// Set the current character size (the bits per byte) to the size in the given
// value.  This is used for both reding and writing, and the size does not
// include the parity bit (if any).
vpr::ReturnStatus SerialPortImplTermios::setCharacterSize(const vpr::SerialTypes::CharacterSizeOption bpb)
{
   struct termios term;
   vpr::ReturnStatus retval;

   if ( (retval = getAttrs(&term)).success() )
   {
      term.c_cflag &= ~CSIZE; // Zero out the bits

      // Set the character size based on the given bits-per-byte value.
      switch ( bpb )
      {
         // 5 bits/byte.
         case SerialTypes::CS_BITS_5:
            term.c_cflag |= CS5;
            break;
            // 6 bits/byte.
         case SerialTypes::CS_BITS_6:
            term.c_cflag |= CS6;
            break;
            // 7 bits/byte.
         case SerialTypes::CS_BITS_7:
            term.c_cflag |= CS7;
            break;
            // 8 bits/byte.
         case SerialTypes::CS_BITS_8:
            term.c_cflag |= CS8;
            break;
      }

      retval = setAttrs(&term, "Could not set character size");
   }

   return retval;
}

// Get the current read state for the port.
bool SerialPortImplTermios::getReadState() const
{
   return getBit(CREAD, SerialPortImplTermios::CFLAG);
}

// Enable or disable the receiver so that bytes can be read from the port.
vpr::ReturnStatus SerialPortImplTermios::setRead(bool flag)
{
   return setBit(CREAD, SerialPortImplTermios::CFLAG, flag,
                 "Could not change read state reading");
}

// Get the current CLOCAL state, if the device is locally attached.
bool SerialPortImplTermios::getLocalAttachState() const
{
   return getBit(CLOCAL, SerialPortImplTermios::CFLAG);
}

// Enable or disable CLOCAL, that is the device is locally attached.
vpr::ReturnStatus SerialPortImplTermios::setLocalAttach(bool flag)
{
   return setBit(CLOCAL, SerialPortImplTermios::CFLAG, flag,
                 "Could not change local attachment");
}

// Get the number of stop bits in use.  This will be either 1 or 2.
vpr::ReturnStatus SerialPortImplTermios::getStopBits(Uint8& num_bits) const
{
   struct termios term;
   vpr::ReturnStatus retval;

   if ( (retval = getAttrs(&term)).success() )
   {
      num_bits = (term.c_cflag & CSTOPB) ? 2 : 1;
   }

   return retval;
}

// Set the number of stop bits to use.  The value must be either 1 or 2.
vpr::ReturnStatus SerialPortImplTermios::setStopBits(const Uint8 num_bits)
{
   struct termios term;
   vpr::ReturnStatus retval;

   if ( (retval = getAttrs(&term)).success() )
   {
      std::string msg;

      switch ( num_bits )
      {
         case 1:
            term.c_cflag &= ~CSTOPB;
            break;
         case 2:
            term.c_cflag |= CSTOPB;
            break;
         default:
            vprDEBUG(vprDBG_ERROR, vprDBG_WARNING_LVL)
               << clrOutBOLD(clrYELLOW, "WARNING:")
               << " [vpr::SerialPortImplTermios::setStopBits()] Stop bits "
               << "may only be set to 1 or 2 on port " << getName()
               << std::endl << vprDEBUG_FLUSH;
            break;
      }

      // Construct the error message to send to setAttrs().
      msg = "Could set not stop bits to ";
      msg += num_bits;

      retval = setAttrs(&term, msg);
   }

   return retval;
}

// Query the canonical input state of the serial port.
bool SerialPortImplTermios::getCanonicalState() const
{
   return getBit(ICANON, SerialPortImplTermios::LFLAG);
}

// Enable or disable canonical input.
vpr::ReturnStatus SerialPortImplTermios::setCanonicalInput(bool flag)
{
   return setBit(ICANON, SerialPortImplTermios::LFLAG, flag,
                 "Could not change canonical input mode");
}

// Get the current state of ignoring bytes with framing errors (other than a
// BREAK) or parity errors.
bool SerialPortImplTermios::getBadByteIgnoreState() const
{
   return getBit(IGNPAR, SerialPortImplTermios::IFLAG);
}

// Enable or disable ignoring of received bytes with framing errors or parity
// errors.
vpr::ReturnStatus SerialPortImplTermios::setBadByteIgnore(bool flag)
{
   return setBit(IGNPAR, SerialPortImplTermios::IFLAG, flag,
                 "Could not change bad byte ignoring");
}

// Get the current state of ignoring BREAK bytes.
bool SerialPortImplTermios::getBreakByteIgnoreState() const
{
   return getBit(IGNBRK, SerialPortImplTermios::IFLAG);
}

// Enable or disable ignoring of received BREAK bytes.
vpr::ReturnStatus SerialPortImplTermios::setBreakByteIgnore(bool flag)
{
   return setBit(IGNBRK, SerialPortImplTermios::IFLAG, flag,
                 "Could not change break byte ignoring");
}

// Get the state of parity checking for input.
bool SerialPortImplTermios::getInputParityCheckState() const
{
   return getBit(IGNPAR, SerialPortImplTermios::IFLAG);
}

// Enable or disable input parity checking.
vpr::ReturnStatus SerialPortImplTermios::setInputParityCheck(bool flag)
{
   return setBit(INPCK, SerialPortImplTermios::IFLAG, flag,
                 "Could not change input parity checking");
}

// Get the current state of bit stripping.  When enabled, input bytes are
// stripped to seven bits.  Otherwise, all eight bits are processed.
bool SerialPortImplTermios::getBitStripState() const
{
   return getBit(ISTRIP, SerialPortImplTermios::IFLAG);
}

// Enable stripping of input bytes to seven bits.
vpr::ReturnStatus SerialPortImplTermios::setBitStripping(bool flag)
{
   return setBit(ISTRIP, SerialPortImplTermios::IFLAG, flag,
                 "Could not change bit stripping setting");
}

// Get the state of start-stop input control.
bool SerialPortImplTermios::getStartStopInputState() const
{
   return getBit(IXOFF, SerialPortImplTermios::IFLAG);
}

// Enable or disable start-stop input control.
vpr::ReturnStatus SerialPortImplTermios::setStartStopInput(bool flag)
{
   return setBit(IXOFF, SerialPortImplTermios::IFLAG, flag,
                 "Could not change start-stop input control");
}

// Get the state of start-stop output control.
bool SerialPortImplTermios::getStartStopOutputState() const
{
   return getBit(IXON, SerialPortImplTermios::IFLAG);
}

// Enable or disable start-stop output control.
vpr::ReturnStatus SerialPortImplTermios::setStartStopOutput(bool flag)
{
   return setBit(IXON, SerialPortImplTermios::IFLAG, flag,
                 "Could not enable start-stop output control");
}

// Get the current state of parity generation for outgoing bytes and parity
// checking for incoming bytes.
bool SerialPortImplTermios::getParityGenerationState() const
{
   return getBit(PARENB, SerialPortImplTermios::CFLAG);
}

// Enable or disable parity generation for outgoing bytes and parity checking
// for incoming bytes.
vpr::ReturnStatus SerialPortImplTermios::setParityGeneration(bool flag)
{
   return setBit(PARENB, SerialPortImplTermios::CFLAG, flag,
                 "Could not change parity generation on outgoing characters");
}

// Enable marking of bytes with parity errors or framing errors (except
// BREAKs).  This is only active if input parity and framing error reporting
// is enabled (see getInputParityCheckState() for more information).  The mark
// is the three-byte sequence \377 \0 X where X is the byte received in error.
// If bit stripping is enabled, a valid \377 byte is passed as the two-byte
// sequence \377 \377.
bool SerialPortImplTermios::getParityErrorMarkingState() const
{
   return getBit(PARMRK, SerialPortImplTermios::IFLAG);
}

// Enable or disable parity error and framing error marking.
vpr::ReturnStatus SerialPortImplTermios::setParityErrorMarking(bool flag)
{
   return setBit(PARMRK, SerialPortImplTermios::IFLAG, flag,
                 "Could not change parity error marking");
}

// Get the current parity checking type (either odd or even).
vpr::SerialTypes::ParityType SerialPortImplTermios::getParity() const
{
   vpr::SerialTypes::ParityType retval;

   // Odd parity if PARODD is set.
   if ( getBit(PARODD, SerialPortImplTermios::CFLAG) )
   {
      retval = SerialTypes::PORT_PARITY_ODD;
   }
   // Even parity if PARODD is not set.
   else
   {
      retval = SerialTypes::PORT_PARITY_EVEN;
   }

   return retval;
}

// Enable odd parity or even parity depending on the argument value.  Odd
// parity corresponds with a value of true, and event parity corresponds with
// a value of false.
vpr::ReturnStatus SerialPortImplTermios::setParity(const vpr::SerialTypes::ParityType& type)
{
   return setBit(PARODD, SerialPortImplTermios::CFLAG, (type == vpr::SerialTypes::PORT_PARITY_ODD),
                 "Could not change parity setting");
}

// Get the current input baud rate.
vpr::ReturnStatus SerialPortImplTermios::getInputBaudRate(vpr::Uint32& rate) const
{
   struct termios term;
   vpr::ReturnStatus retval;

   if ( (retval = getAttrs(&term)).success() )
   {
      speed_t baud_rate;

      baud_rate = cfgetispeed(&term);
      rate      = baudToInt(baud_rate);
   }

   return retval;
}

// Set the current input baud rate.
vpr::ReturnStatus SerialPortImplTermios::setInputBaudRate(const vpr::Uint32& baud)
{
   struct termios term;
   vpr::ReturnStatus retval;

   if ( (retval = getAttrs(&term)).success() )
   {
      speed_t new_rate;

      new_rate = intToBaud(baud);

      vprDEBUG(vprDBG_ALL, vprDBG_VERB_LVL)
         << "SerialPortImplTermios::setInputBaudRate(): Setting input baud "
         << "rate to " << new_rate << " (converted from " << baud << ")\n"
         << vprDEBUG_FLUSH;

      if ( cfsetispeed(&term, new_rate) == -1 )
      {
         vprDEBUG(vprDBG_ERROR, vprDBG_WARNING_LVL)
            << clrOutBOLD(clrYELLOW, "WARNING:")
            << " [vpr::SerialPortImplTermios::setInputBaudRate()] "
            << "Failed to set the input baud rate to " << baud << " on port "
            << getName() << std::endl << vprDEBUG_FLUSH;
         vprDEBUG_NEXT(vprDBG_ERROR, vprDBG_WARNING_LVL)
            << strerror(errno) << std::endl << vprDEBUG_FLUSH;
         retval.setCode(ReturnStatus::Fail);
      }
      else
      {
         std::string msg;

         msg     = "Failed to set the input baud rate to ";
         msg    += baud;
         retval  = setAttrs(&term, msg);
      }
   }

   return retval;
}

// Get the current output baud rate.
vpr::ReturnStatus SerialPortImplTermios::getOutputBaudRate(vpr::Uint32& rate) const
{
   struct termios term;
   vpr::ReturnStatus retval;

   if ( (retval = getAttrs(&term)).success() )
   {
      speed_t baud_rate;

      baud_rate = cfgetospeed(&term);
      rate      = baudToInt(baud_rate);
   }

   return retval;
}

// Set the current output baud rate.
vpr::ReturnStatus SerialPortImplTermios::setOutputBaudRate(const vpr::Uint32& baud)
{
   struct termios term;
   vpr::ReturnStatus retval;

   if ( (retval = getAttrs(&term)).success() )
   {
      speed_t new_rate;

      new_rate = intToBaud(baud);

      vprDEBUG(vprDBG_ALL, vprDBG_VERB_LVL)
         << "SerialPortImplTermios::setInputBaudRate(): Setting output baud "
         << "rate to " << new_rate << " (converted from " << baud << ")\n"
         << vprDEBUG_FLUSH;

      if ( cfsetospeed(&term, new_rate) == -1 )
      {
         vprDEBUG(vprDBG_ERROR, vprDBG_WARNING_LVL)
            << clrOutBOLD(clrYELLOW, "WARNING:")
            << " [vpr::SerialPortImplTermios::setOutputBaudRate()] "
            << "Failed to set the output baud rate to " << baud << " on port "
            << getName() << std::endl << vprDEBUG_FLUSH;
         vprDEBUG_NEXT(vprDBG_ERROR, vprDBG_WARNING_LVL)
            << strerror(errno) << std::endl << vprDEBUG_FLUSH;
         retval.setCode(ReturnStatus::Fail);
      }
      else
      {
         std::string msg;

         msg     = "Failed to set the output baud rate to ";
         msg    += baud;
         retval  = setAttrs(&term, msg);
      }
   }

   return retval;
}

// Wait for all output to be transmitted.
vpr::ReturnStatus SerialPortImplTermios::drainOutput()
{
   vprASSERT(mHandle->mFdesc != -1 && "The port may not be open");
   vpr::ReturnStatus retval;

   if ( tcdrain(mHandle->mFdesc) == -1 )
   {
      vprDEBUG(vprDBG_ERROR, vprDBG_WARNING_LVL)
         << clrOutBOLD(clrYELLOW, "WARNING:")
         << " [vpr::SerialPortImplTermios::drainOutput()] "
         << "Failed drain output on port " << getName() << std::endl
         << vprDEBUG_FLUSH;
      vprDEBUG_NEXT(vprDBG_ERROR, vprDBG_WARNING_LVL)
         << strerror(errno) << std::endl << vprDEBUG_FLUSH;
      retval.setCode(ReturnStatus::Fail);
   }

   return retval;
}

// Alter the input or output flow control.  Based on the
// vpr::SerialTypes::FlowActionOption argument, output can be suspended and
// restarted or the terminal device can be told to stop or to resume sending
// data.
vpr::ReturnStatus SerialPortImplTermios::controlFlow(SerialTypes::FlowActionOption opt)
{
   vprASSERT(mHandle->mFdesc != -1 && "The port may not be open");
   int action = -1;
   vpr::ReturnStatus retval;

   switch ( opt )
   {
      case SerialTypes::OUTPUT_OFF:
         action = TCOOFF;
         break;
      case SerialTypes::OUTPUT_ON:
         action = TCOON;
         break;
      case SerialTypes::INPUT_OFF:
         action = TCIOFF;
         break;
      case SerialTypes::INPUT_ON:
         action = TCION;
         break;
   }

   if ( tcflow(mHandle->mFdesc, action) == -1 )
   {
      vprDEBUG(vprDBG_ERROR, vprDBG_WARNING_LVL)
         << clrOutBOLD(clrYELLOW, "WARNING:")
         << " [vpr::SerialPortImplTermios::controlFlow()] "
         << "Failed alter flow control on port " << getName() << std::endl
         << vprDEBUG_FLUSH;
      vprDEBUG_NEXT(vprDBG_ERROR, vprDBG_WARNING_LVL)
         << strerror(errno) << std::endl << vprDEBUG_FLUSH;
      retval.setCode(ReturnStatus::Fail);
   }

   return retval;
}

bool SerialPortImplTermios::getHardwareFlowControlState() const
{
#ifdef VPR_OS_IRIX
   return getBit(CNEW_RTSCTS, SerialPortImplTermios::CFLAG);
#else
   return getBit(CRTSCTS, SerialPortImplTermios::CFLAG);
#endif
}

vpr::ReturnStatus SerialPortImplTermios::setHardwareFlowControl(bool flag)
{
#ifdef VPR_OS_IRIX
   return setBit(CNEW_RTSCTS, SerialPortImplTermios::CFLAG, flag,
                 "Could not change hardware flow control");
#else
   return setBit(CRTSCTS, SerialPortImplTermios::CFLAG, flag,
                 "Could not change hardware flow control");
#endif
}

// Discard either the input buffer (unread data received from the terminal
// device) or the output buffer (data written but not yet transmitted to the
// terminal device).  The argument tells which queue (or queues) to flush.
vpr::ReturnStatus SerialPortImplTermios::flushQueue(SerialTypes::FlushQueueOption vpr_queue)
{
   vprASSERT(mHandle->mFdesc != -1 && "The port may not be open");
   int queue = -1;
   vpr::ReturnStatus retval;

   switch ( vpr_queue )
   {
      case SerialTypes::INPUT_QUEUE:
         queue = TCIFLUSH;
         break;
      case SerialTypes::OUTPUT_QUEUE:
         queue = TCOFLUSH;
         break;
      case SerialTypes::IO_QUEUES:
         queue = TCIOFLUSH;
         break;
   }

   if ( tcflush(mHandle->mFdesc, queue) == -1 )
   {
      std::string queue_name;

      switch ( vpr_queue )
      {
         case SerialTypes::INPUT_QUEUE:
            queue_name = "input queue";
            break;
         case SerialTypes::OUTPUT_QUEUE:
            queue_name = "output queue";
            break;
         case SerialTypes::IO_QUEUES:
            queue_name = "input and output queues";
            break;
      }

      vprDEBUG(vprDBG_ERROR, vprDBG_WARNING_LVL)
         << clrOutBOLD(clrYELLOW, "WARNING:")
         << " [vpr::SerialPortImplTermios::flushQueue()] "
         << "Failed to flush " << queue_name << " on port " << getName()
         << std::endl << vprDEBUG_FLUSH;
      vprDEBUG_NEXT(vprDBG_ERROR, vprDBG_WARNING_LVL)
         << strerror(errno) << std::endl << vprDEBUG_FLUSH;
      retval.setCode(ReturnStatus::Fail);
   }

   return retval;
}

// Transmit a continuous stream of zero bits for the given duration.  If the
// argument is 0, the transmission will last between 0.25 and 0.5 seconds.
// Otherwise, the duration specfies the number of seconds to send the zero bit
// stream.
vpr::ReturnStatus SerialPortImplTermios::sendBreak(const Int32 duration)
{
   vprASSERT(mHandle->mFdesc != -1 && "The port may not be open");
   vpr::ReturnStatus retval;

   if ( tcsendbreak(mHandle->mFdesc, duration) == -1 )
   {
      vprDEBUG(vprDBG_ERROR, vprDBG_WARNING_LVL)
         << clrOutBOLD(clrYELLOW, "WARNING:")
         << " [vpr::SerialPortImplTermios::sendBreak()] "
         << "Failed to send break on port " << getName() << std::endl
         << vprDEBUG_FLUSH;
      vprDEBUG_NEXT(vprDBG_ERROR, vprDBG_WARNING_LVL)
         << strerror(errno) << std::endl << vprDEBUG_FLUSH;
      retval.setCode(ReturnStatus::Fail);
   }

   return retval;
}


// Modem line methods
/* modem lines */
/* Linux definitions
#define TIOCM_LE	0x001
#define TIOCM_DTR	0x002
#define TIOCM_RTS	0x004
#define TIOCM_ST	0x008
#define TIOCM_SR	0x010
#define TIOCM_CTS	0x020
#define TIOCM_CAR	0x040
#define TIOCM_RNG	0x080
#define TIOCM_DSR	0x100
#define TIOCM_CD	TIOCM_CAR
#define TIOCM_RI	TIOCM_RNG
*/

/**
* Return the status of the carrier detect signal.
* @return - May be platform dependent, but will at least be as follows.
*           0 - not high, 1 - high, -1 - Not supported
*/
int SerialPortImplTermios::getCarrierDetect() const
{
   if(getLineFlag(TIOCM_CAR))
      return 1;
   else
      return 0;
}

/**
* Return the status of the data set ready line.
* @return - May be platform dependent, but will at least be as follows.
*           0 - not high, 1 - high, -1 - Not supported
*/
int SerialPortImplTermios::getDataSetReady() const
{
   if(getLineFlag(TIOCM_DSR))
   {
      return 1;
   }
   else
   {
      return 0;
   }
}

/**
* Return the status of the clear to send.
* @return - May be platform dependent, but will at least be as follows.
*           0 - not high, 1 - high, -1 - Not supported
*/
int SerialPortImplTermios::getClearToSend() const
{
   if(getLineFlag(TIOCM_CTS))
   {
      return 1;
   }
   else
   {
      return 0;
   }
}

/**
* Return the status of the ring indicator line.
* @return - May be platform dependent, but will at least be as follows.
*           0 - not high, 1 - high, -1 - Not supported
*/
int SerialPortImplTermios::getRingIndicator() const
{
   if(getLineFlag(TIOCM_RI))
   {
      return 1;
   }
   else
   {
      return 0;
   }
}

/** Set the data terminal ready line. */
vpr::ReturnStatus SerialPortImplTermios::setDataTerminalReady(bool val)
{
   return setLineFlag(TIOCM_DTR, val);
}

/** Set the ready to send line */
vpr::ReturnStatus SerialPortImplTermios::setRequestToSend(bool val)
{
   return setLineFlag(TIOCM_RTS, val);
}

// ============================================================================
// Protected methods.
// ============================================================================

// Set the control character at the given index to the given value.
void SerialPortImplTermios::setControlCharacter(const Uint32 index,
                                                const Uint8 value)
{
   struct termios term;

   if ( getAttrs(&term).success() )
   {
      if ( index < NCCS )
      {
         std::string msg;

         term.c_cc[index] = value;

         msg  = "Could not set control character ";
         msg += index;
         msg += " to ";
         msg += value;
         setAttrs(&term, msg);
      }
      else
      {
         vprDEBUG(vprDBG_ERROR, vprDBG_WARNING_LVL)
            << "[vpr::SerialPortImplTermios::setControlCharacter()] Index "
            << index << " too large for control character array\n"
            << vprDEBUG_FLUSH;
      }
   }
}

// Get the value of the control character at the given index.
Uint8 SerialPortImplTermios::getControlCharacter(const Uint32 index) const
{
   struct termios term;
   Uint8 retval;

   retval = 0;

   if ( getAttrs(&term).success() )
   {
      if ( index < NCCS )
      {
         retval = term.c_cc[index];
      }
      else
      {
         vprDEBUG(vprDBG_ERROR, vprDBG_WARNING_LVL)
            << "[vpr::SerialPortImplTermios::getControlCharacter()] Index "
            << index << " too large for control character array\n"
            << vprDEBUG_FLUSH;
      }
   }

   return retval;
}

vpr::ReturnStatus SerialPortImplTermios::getAttrs(struct termios* term) const
{
   vprASSERT(mHandle->mFdesc != -1 && "The port may not be open");
   vpr::ReturnStatus retval;

   if ( tcgetattr(mHandle->mFdesc, term) == -1 )
   {
      vprDEBUG(vprDBG_ERROR, vprDBG_WARNING_LVL)
         << clrOutBOLD(clrYELLOW, "WARNING:")
         << " [vpr::SerialPortImplTermios::getAttrs()] "
         << "Could not get attributes for port " << getName() << std::endl
         << vprDEBUG_FLUSH;
      vprDEBUG_NEXT(vprDBG_ERROR, vprDBG_WARNING_LVL)
         << strerror(errno) << std::endl << vprDEBUG_FLUSH;
      retval.setCode(ReturnStatus::Fail);
   }

   return retval;
}

vpr::ReturnStatus SerialPortImplTermios::setAttrs(struct termios* term,
                                                  const char* err_msg,
                                                  const bool print_sys_err)
{
   vprASSERT(mHandle->mFdesc != -1 && "The port may not be open");
   vprASSERT(term != NULL);
   vprASSERT(err_msg != NULL);

   vpr::ReturnStatus retval;

#ifdef VPR_DEBUG
   vprDEBUG(vprDBG_ALL, vprDBG_STATE_LVL)
      << "[vpr::SerialPortImplTermios] term->c_iflag: " << term->c_iflag
      << std::endl << vprDEBUG_FLUSH;
   vprDEBUG(vprDBG_ALL, vprDBG_STATE_LVL)
      << "[vpr::SerialPortImplTermios] term->c_oflag: " << term->c_oflag
      << std::endl << vprDEBUG_FLUSH;
   vprDEBUG(vprDBG_ALL, vprDBG_STATE_LVL)
      << "[vpr::SerialPortImplTermios] term->c_cflag: " << term->c_cflag
      << std::endl << vprDEBUG_FLUSH;
   vprDEBUG(vprDBG_ALL, vprDBG_STATE_LVL)
      << "[vpr::SerialPortImplTermios] term->c_lflag: " << term->c_lflag
      << std::endl << vprDEBUG_FLUSH;
   vprDEBUG(vprDBG_ALL, vprDBG_STATE_LVL)
      << "[vpr::SerialPortImplTermios] term->c_ispeed: " << cfgetispeed(term)
      << std::endl << vprDEBUG_FLUSH;
   vprDEBUG(vprDBG_ALL, vprDBG_STATE_LVL)
      << "[vpr::SerialPortImplTermios] term->c_ospeed: " << cfgetospeed(term)
      << std::endl << vprDEBUG_FLUSH;
   vprDEBUG(vprDBG_ALL, vprDBG_STATE_LVL)
      << "[vpr::SerialPortImplTermios] term->c_cc[]:" << vprDEBUG_FLUSH;
   for ( int i = 0; i < NCCS; i++ )
   {
      vprDEBUG_CONT(vprDBG_ALL, vprDBG_STATE_LVL)
         << " " << (unsigned int) term->c_cc[i] << vprDEBUG_FLUSH;
   }
   vprDEBUG_CONT(vprDBG_ALL, vprDBG_STATE_LVL) << std::endl << vprDEBUG_FLUSH;
#endif

   if ( tcsetattr(mHandle->mFdesc, mActions, term) == -1 )
   {
      vprDEBUG(vprDBG_ERROR, vprDBG_WARNING_LVL)
         << " [vpr::SerialPortImplTermios::setAttrs()] " << err_msg
         << " (port '" << getName() << "'\n" << vprDEBUG_FLUSH;

      if ( print_sys_err )
      {
         vprDEBUG_NEXT(vprDBG_ERROR, vprDBG_WARNING_LVL)
            << strerror(errno) << std::endl << vprDEBUG_FLUSH;
      }

      retval.setCode(ReturnStatus::Fail);
   }

   return retval;
}

vpr::ReturnStatus SerialPortImplTermios::setAttrs(struct termios* term,
                                                  const std::string& err_msg,
                                                  const bool print_sys_err)
{
   return setAttrs(term, err_msg.c_str(), print_sys_err);
}

bool SerialPortImplTermios::getBit(const tcflag_t bit,
                                   SerialPortImplTermios::TermFlag flag) const
{
   struct termios term;
   bool retval = false;

   if ( getAttrs(&term).success() )
   {
      switch ( flag )
      {
         case IFLAG:
            retval = ((term.c_iflag & bit) != 0);
         case OFLAG:
            retval = ((term.c_oflag & bit) != 0);
         case CFLAG:
            retval = ((term.c_cflag & bit) != 0);
         case LFLAG:
            retval = ((term.c_lflag & bit) != 0);
      }
   }

   return retval;
}

vpr::ReturnStatus SerialPortImplTermios::setBit(const tcflag_t bit,
                                                SerialPortImplTermios::TermFlag flag,
                                                bool enable,
                                                const std::string& err_msg,
                                                const bool print_sys_err)
{
   struct termios term;
   vpr::ReturnStatus retval;

   if ( (retval = getAttrs(&term)).success() )
   {
      // Set the bit for this port.
      if ( enable )
      {
         switch ( flag )
         {
            case IFLAG:
               term.c_iflag |= bit;
               break;
            case OFLAG:
               term.c_oflag |= bit;
               break;
            case CFLAG:
               term.c_cflag |= bit;
               break;
            case LFLAG:
               term.c_lflag |= bit;
               break;
         }
      }
      // Reset the bit for this port.
      else
      {
         switch ( flag )
         {
            case IFLAG:
               term.c_iflag &= ~bit;
               break;
            case OFLAG:
               term.c_oflag &= ~bit;
               break;
            case CFLAG:
               term.c_cflag &= ~bit;
               break;
            case LFLAG:
               term.c_lflag &= ~bit;
               break;
         }
      }

      retval = setAttrs(&term, err_msg, print_sys_err);
   }

   return retval;
}

vpr::ReturnStatus SerialPortImplTermios::setLineFlag(Uint16 flag, bool val)
{
   vprASSERT(mHandle->mFdesc != -1 && "The port may not be open");
   vprASSERT( ((TIOCM_DTR == flag) || (TIOCM_RTS == flag)) && "Tried to set line flag that can't be set");

   int line_status;        // current flags

   if(ioctl(mHandle->mFdesc, TIOCMGET, &line_status) == -1)
   {
      return vpr::ReturnStatus::Fail;
   }

   if(val)
      line_status |= flag;
   else
      line_status &= ~flag;

   if(ioctl(mHandle->mFdesc, TIOCMSET, &line_status) == -1)
   {
      return vpr::ReturnStatus::Fail;
   }
   //tcflush(port->desc, TCIFLUSH);
   flushQueue(vpr::SerialTypes::IO_QUEUES);  // Flush queues for goodness
   	
   return vpr::ReturnStatus::Succeed;
}

bool SerialPortImplTermios::getLineFlag(Uint16 flag) const
{
   vprASSERT(mHandle->mFdesc != -1 && "The port may not be open");
   int line_status;

   if(ioctl(mHandle->mFdesc, TIOCMGET, &line_status) == -1)
   {
      return vpr::ReturnStatus::Fail;
   }

   return (line_status & flag);
}

// Convert a termios baud rate to its corresponding integer value.
Uint32 SerialPortImplTermios::baudToInt(const speed_t baud_rate) const
{
   Uint32 retval;

   switch ( baud_rate )
   {
      case B50:
         retval = 50;
         break;
      case B75:
         retval = 75;
         break;
      case B110:
         retval = 110;
         break;
      case B134:
         retval = 134;
         break;
      case B150:
         retval = 150;
         break;
      case B200:
         retval = 200;
         break;
      case B300:
         retval = 300;
         break;
      case B600:
         retval = 600;
         break;
      case B1200:
         retval = 1200;
         break;
      case B1800:
         retval = 1800;
         break;
      case B2400:
         retval = 2400;
         break;
      case B4800:
         retval = 4800;
         break;
      case B9600:
         retval = 9600;
         break;
      case B19200:
         retval = 19200;
         break;
      case B38400:
         retval = 38400;
         break;
      case B57600:
         retval = 57600;
         break;
#ifdef B76800
      case B76800:
         retval = 76800;
         break;
#endif
      case B115200:
         retval = 115200;
         break;
#ifdef B230400
      case B230400:
         retval = 230400;
         break;
#endif
#ifdef B460800
      case B460800:
         retval = 460800;
         break;
#endif
#ifdef B921600
      case B921600:
         retval = 921600;
         break;
#endif
      default:
         retval = baud_rate;
         break;
   }

   return retval;
}

// Convert an integer baud rate to the corresponding termios rate constant.
speed_t SerialPortImplTermios::intToBaud(const Uint32 speed_int) const
{
   speed_t rate;

   switch ( speed_int )
   {
      case 50:
         rate = B50;
         break;
      case 75:
         rate = B75;
         break;
      case 110:
         rate = B110;
         break;
      case 134:
         rate = B134;
         break;
      case 150:
         rate = B150;
         break;
      case 200:
         rate = B200;
         break;
      case 300:
         rate = B300;
         break;
      case 600:
         rate = B600;
         break;
      case 1200:
         rate = B1200;
         break;
      case 1800:
         rate = B1800;
         break;
      case 2400:
         rate = B2400;
         break;
      case 4800:
         rate = B4800;
         break;
      case 9600:
         rate = B9600;
         break;
      case 19200:
         rate = B19200;
         break;
      case 38400:
         rate = B38400;
         break;
      case 57600:
         rate = B57600;
         break;
#ifdef B76800
      case 76800:
         rate = B76800;
         break;
#endif
      case 115200:
         rate = B115200;
         break;
#ifdef B230400
      case 230400:
         rate = B230400;
         break;
#endif
#ifdef B460800
      case 460800:
         rate = B460800;
         break;
#endif
#ifdef B921600
      case 921600:
         rate = B921600;
         break;
#endif
      default:
         rate = speed_int;
         break;
   }

   return rate;
}

} // End of vpr namespace
