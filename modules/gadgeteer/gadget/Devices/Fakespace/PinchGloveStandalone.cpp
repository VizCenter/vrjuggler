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


//===============================================================
// PinchGloveStandalone
//
// Purpose:
//    software interface to Fakespace Pinchglove hardware
//
// Author:
// Kevin Meinert
//
// NOTES:
//      The long function names greatly decrease the
//       ambiguity of what the functions do ... sorry. :)
//
// Date: 1-23-99
//===============================================================

/*#include <string.h> // for strcpy();
#include <stdio.h>
#include <stdlib.h>
#include <iostream>        // for std::cout
// #include <fstream>         // for ifstream
#include <string.h>

#include <vpr/Util/Assert.h>        // for assert
*/

#include <gadget/gadgetConfig.h>
#include <stdio.h>
#include <vpr/vpr.h>
#include <vpr/System.h>
#include <gadget/Devices/Fakespace/PinchGloveStandalone.h>

const std::string PinchGloveStandalone::mOpenHandString = "00000.00000";

///////////////////////////////////////////////////////////
// Public Methods
///////////////////////////////////////////////////////////

// Constructor
PinchGloveStandalone::PinchGloveStandalone()
{
   mGestureString = mOpenHandString;
   mPreviousGestureString = mOpenHandString;
   strncpy( mGestureStringTemp, mOpenHandString.data(), 12 );
   first = 1;
}

PinchGloveStandalone::~PinchGloveStandalone()
{
   if ( port != NULL )
   {
      port->close();
      delete port;
      port = NULL;
   }
}

// Connect to the pinch glove hardware
bool PinchGloveStandalone::connectToHardware(const std::string& ttyPort, int mBaudRate)
{
   std::cout<<"\n[pinch] Connecting To Fakespace Hardware\n"<<std::flush;
   int result = mConnectToHardware( ttyPort , mBaudRate);
   if ( result == 1 )
   {
      std::cout<<"[pinch] Connected to pinch glove hardware on port "<<ttyPort<<"\n"<<std::flush;
   }
   else
   {
      std::cout<<"[pinch] connectToHardware(\""<<ttyPort<<"\") returned "<<(result == 1 ? "true":"false")<<"\n"<<std::flush;
   }
   std::cout<<"\n"<<std::flush;
   return result == 1 ? true:false;
}

// get the last sampled string,
// NOTE: call ReSampleData to get most current pinch data.
void PinchGloveStandalone::getSampledString(std::string& gestureString)
{
   gestureString = mGestureString;
}

// call ReSampleData to get most current pinch data.
void PinchGloveStandalone::updateStringFromHardware()
{
   // make a copy into the previous string.
   mPreviousGestureString = mGestureString;

   // get the current string from the pinch hardware
   // If there is no _new_ string, then tempString will not be modified.
   // NOTE: this is why we have to save the old value in tempString
   mGetStringFromHardware( (char*) mGestureStringTemp );
   mGestureString = mGestureStringTemp;
}



///////////////////////////////////////////////////////////
// Private methods
///////////////////////////////////////////////////////////

// send to hardware methods:

int PinchGloveStandalone::mConnectToHardware(const std::string& ttyPort, int baud)
{
   const int BUFFER_LEN = 100;
   unsigned char buf[BUFFER_LEN];
   int cnt;
   //long int baud;

   //CREATE A NEW SERIAL PORT
   port = new vpr::SerialPort(ttyPort);
   port->setOpenReadWrite();

   if ( !port->open().success() )
   {
      std::cout<<"[pinch] Port ("<<ttyPort<<") open failed\n"<<std::flush;
      port->close();
      return 0;
   }
   else
   {
      std::cout<<"[pinch] Port ("<<ttyPort<<") open success\n"<<std::flush;
      //baud = 9600;
      port->clearAll();
      port->enableLocalAttach();
      port->enableBreakByteIgnore();
      port->enableRead();
      port->setBufferSize(1);
      port->setOutputBaudRate(baud); // Put me before input to be safe
      port->setInputBaudRate(baud);
      port->setCharacterSize(vpr::SerialTypes::CS_BITS_8);
      std::cout<<"[pinch] Port ("<<ttyPort<<") successfully changed the port settings\n"<<std::flush;
   }

   vpr::System::usleep(450000);

   port->flushQueue(vpr::SerialTypes::IO_QUEUES);

   /* Turn time stamps on */
   mSendCommandToHardware("T1", buf);
   if ( buf[1] != '1' )
   {
      std::cout<<"[pinch] Could not turn time stamps on, result should == '1' but result == '"<<buf[1]<<"'\n";
      return 0;
   }
   else
   {   //DELETE
      std::cout<<"[pinch] Turned time stamps on.\n";
   }
   /* Version compatability */
   vpr::System::usleep(450000);
   mSendCommandToHardware("V1", buf);
   if ( buf[1] != '1' )
   {
      std::cout << "[pinch] Could not set to version 1 formatting, result should == '1' but result == '"<<buf[1]<<"'\n";
      return 0;
   }
   else
   {
      printf("[pinch] Set to version 1 formatting\n");
   }
   /* Get the configuration information and print it */
   printf("[pinch] Configuration:\n");
   vpr::System::usleep(450000);
   cnt = mSendCommandToHardware("CP", buf);
   buf[cnt-1] = 0; /* get rid of 0x8F */
   printf("\t%s\n",&buf[1]);

   vpr::System::usleep(450000);
   cnt = mSendCommandToHardware("CL", buf);
   buf[cnt-1] = 0; /* get rid of 0x8F */
   printf("\t%s\n",&buf[1]);

   vpr::System::usleep(450000);
   cnt = mSendCommandToHardware("CR", buf);
   buf[cnt-1] = 0; /* get rid of 0x8F */
   printf("\t%s\n",&buf[1]);

   vpr::System::usleep(450000);
   cnt = mSendCommandToHardware("CT", buf);
   buf[cnt-1] = 0; /* get rid of 0x8F */
   printf("\t%s\n",&buf[1]);

   return 1;
}

int PinchGloveStandalone::mSendCommandToHardware(const char* const command, unsigned char *reply)
{
   vpr::Uint32 written;
   char buf[100];
   first = 1;
   vpr::ReturnStatus status;

   ///MAKE FIRST GLOBAL/MEMBER VARIABLE

   //This code flushes the command buffer on the PinchGlove side. It is only once
   if ( first )
   {
      first = 0;
      port->write("*", 1, written);
      vpr::System::usleep(450000);
      status = port->read(&buf[0], 3, written, vpr::Interval::NoWait);
      if ( status!=vpr::ReturnStatus::Succeed )
      {
         port->write("*", 1, written);
         vpr::System::usleep(450000);
         port->read(&buf[0], 3,written, vpr::Interval::NoWait);
      }
   }


   // Send the 2 byte command by sending each byte seperately and flushing port after each

   port->write(&command[0], 1, written);
   port->flushQueue(vpr::SerialTypes::OUTPUT_QUEUE);
   vpr::System::usleep(450000);

   port->write(&command[1], 1, written);
   port->flushQueue(vpr::SerialTypes::OUTPUT_QUEUE);
   vpr::System::usleep(450000);

   return(mReadRecordsFromHardware(100,reply));
}

//// Read from hardware methods:

int PinchGloveStandalone::mReadRecordsFromHardware(const int& rec_max_len, unsigned char *records)
{
   vpr::Uint32 written;
   int numbytes = 0;
   unsigned char buf[2048];
   vpr::ReturnStatus status;

#define START_BYTE_DATA 0x80
#define START_BYTE_DATA_TS 0x81
#define START_BYTE_TEXT 0x82
#define END_BYTE 0x8F

   records[0] = 0;
   written = 0;

   //vpr::System::usleep(150000);

   status = port->read(&buf[0], 1, written, vpr::Interval::NoWait);

   while ( (records[numbytes-written] != END_BYTE) && status == vpr::ReturnStatus::Succeed )
   {
      if ( (buf[0] == START_BYTE_DATA) || (buf[0] == START_BYTE_DATA_TS) || (buf[0] == START_BYTE_TEXT) )
      {
         records[numbytes++] = buf[0];
         do
         {
            written=0;
            port->read(&records[numbytes], 1,written);
            numbytes += written;
         }while ( (records[numbytes-written] != END_BYTE) );
      }
      //vpr::System::usleep(150000);
   }
   return numbytes;
}

void PinchGloveStandalone::mGetStringFromHardware( char* gesture)
{
   const int BUFFER_LEN = 100;
   unsigned char rec[BUFFER_LEN];
   char data[BUFFER_LEN];
   static char nges = '0';
   int i, num;
   int touch_count;

   num = mReadRecordsFromHardware( BUFFER_LEN, rec );
   // if anything outputs from mReadRecordsFromHardware(), then...
   if ( num )
   {
      strncpy( gesture, mOpenHandString.data(), 12 );

      switch ( rec[0] )
      {
         case 0x80:
         case 0x81:
            if ( rec[0] == 0x80 )
            {
               for ( i=1; i<num-2; ++i )
               {
                  data[i-1] = rec[i];
               }
               touch_count = (num-2)/2;
            }

            else
            {
               for ( i=1; i<num-3; ++i )
               {
                  data[i-1] = rec[i];
               }
               touch_count = (num-4)/2;
            }


            for ( i=0; i<touch_count; ++i )
            {
               /* for debug onlu */
               /*printf(" %02X.%02X\n",data[2*i],data[2*i+1]); */
               switch ( i )
               {
                  case 0: nges = '1'; break;
                  case 1: nges = '2'; break;
                  case 2: nges = '3'; break;
                  case 3: nges = '4'; break;
                  case 4: nges = '5'; break;
                  default: nges = '0';
               }

               /* left hand */
               switch ( data[2*i] )
               {
                  case 0x10 :  gesture[0] = nges; break;
                  case 0x01 :  gesture[4] = nges; break;
                  case 0x11 :  gesture[0] = nges; gesture[4] = nges; break;
                  case 0x02 :  gesture[3] = nges; break;
                  case 0x12 :  gesture[0] = nges; gesture[3] = nges; break;
                  case 0x03 :  gesture[3] = nges; gesture[4] = nges; break;
                  case 0x13 :  gesture[0] = nges; gesture[3] = nges; gesture[4] = nges; break;
                  case 0x04 :  gesture[2] = nges; break;
                  case 0x14 :  gesture[0] = nges; gesture[2] = nges; break;
                  case 0x05 :  gesture[2] = nges; gesture[4] = nges; break;
                  case 0x15 :  gesture[0] = nges; gesture[2] = nges; gesture[4] = nges; break;
                  case 0x06 :  gesture[2] = nges; gesture[3] = nges; break;
                  case 0x16 :  gesture[0] = nges; gesture[2] = nges; gesture[3] = nges; break;
                  case 0x07 :  gesture[2] = nges; gesture[3] = nges; gesture[4] = nges; break;
                  case 0x17 :  gesture[0] = nges; gesture[2] = nges; gesture[3] = nges; gesture[4] = nges; break;
                  case 0x08 :  gesture[1] = nges; break;
                  case 0x18 :  gesture[0] = nges; gesture[1] = nges; break;
                  case 0x09 :  gesture[1] = nges; gesture[4] = nges; break;
                  case 0x19 :  gesture[0] = nges; gesture[1] = nges; gesture[4] = nges; break;
                  case 0x0A :  gesture[1] = nges; gesture[3] = nges; break;
                  case 0x1A :  gesture[0] = nges; gesture[1] = nges; gesture[3] = nges; break;
                  case 0x0B :  gesture[1] = nges; gesture[3] = nges; gesture[4] = nges; break;
                  case 0x1B :  gesture[0] = nges; gesture[1] = nges; gesture[3] = nges; gesture[4] = nges; break;
                  case 0x0C :  gesture[1] = nges; gesture[2] = nges; break;
                  case 0x1C :  gesture[0] = nges; gesture[1] = nges; gesture[2] = nges; break;
                  case 0x0D :  gesture[1] = nges; gesture[2] = nges; gesture[4] = nges; break;
                  case 0x1D :  gesture[0] = nges; gesture[1] = nges; gesture[2] = nges; gesture[4] = nges; break;
                  case 0x0E :  gesture[1] = nges; gesture[2] = nges; gesture[3] = nges; break;
                  case 0x1E :  gesture[0] = nges; gesture[1] = nges; gesture[2] = nges; gesture[3] = nges; break;
                  case 0x0F :  gesture[1] = nges; gesture[2] = nges; gesture[3] = nges; gesture[4] = nges; break;
                  case 0x1F :  gesture[0] = nges; gesture[1] = nges; gesture[2] = nges; gesture[3] = nges; gesture[4] = nges; break;
                  default: break;
               }

               switch ( data[2*i+1] ) /* right hand */
               {
                  case 0x10 :  gesture[6] = nges; break;
                  case 0x01 :  gesture[10] = nges; break;
                  case 0x11 :  gesture[6] = nges; gesture[10] = nges; break;
                  case 0x02 :  gesture[9] = nges; break;
                  case 0x12 :  gesture[6] = nges; gesture[9] = nges; break;
                  case 0x03 :  gesture[9] = nges; gesture[10] = nges; break;
                  case 0x13 :  gesture[6] = nges; gesture[9] = nges; gesture[10] = nges; break;
                  case 0x04 :  gesture[8] = nges; break;
                  case 0x14 :  gesture[6] = nges; gesture[8] = nges; break;
                  case 0x05 :  gesture[8] = nges; gesture[10] = nges; break;
                  case 0x15 :  gesture[6] = nges; gesture[8] = nges; gesture[10] = nges; break;
                  case 0x06 :  gesture[8] = nges; gesture[9] = nges; break;
                  case 0x16 :  gesture[1] = nges; gesture[8] = nges; gesture[9] = nges; break;
                  case 0x07 :  gesture[8] = nges; gesture[9] = nges; gesture[10] = nges; break;
                  case 0x17 :  gesture[6] = nges; gesture[8] = nges; gesture[9] = nges; gesture[10] = nges; break;
                  case 0x08 :  gesture[7] = nges; break;
                  case 0x18 :  gesture[6] = nges; gesture[7] = nges; break;
                  case 0x09 :  gesture[7] = nges; gesture[10] = nges; break;
                  case 0x19 :  gesture[1] = nges; gesture[7] = nges; gesture[10] = nges; break;
                  case 0x0A :  gesture[7] = nges; gesture[9] = nges; break;
                  case 0x1A :  gesture[6] = nges; gesture[7] = nges; gesture[9] = nges; break;
                  case 0x0B :  gesture[7] = nges; gesture[9] = nges; gesture[10] = nges; break;
                  case 0x1B :  gesture[6] = nges; gesture[7] = nges; gesture[9] = nges; gesture[10] = nges; break;
                  case 0x0C :  gesture[7] = nges; gesture[8] = nges; break;
                  case 0x1C :  gesture[6] = nges; gesture[7] = nges; gesture[8] = nges; break;
                  case 0x0D :  gesture[7] = nges; gesture[8] = nges; gesture[10] = nges; break;
                  case 0x1D :  gesture[6] = nges; gesture[7] = nges; gesture[8] = nges; gesture[10] = nges; break;
                  case 0x0E :  gesture[7] = nges; gesture[8] = nges; gesture[9] = nges; break;
                  case 0x1E :  gesture[6] = nges; gesture[7] = nges; gesture[8] = nges; gesture[9] = nges; break;
                  case 0x0F :  gesture[7] = nges; gesture[8] = nges; gesture[9] = nges; gesture[10] = nges; break;
                  case 0x1F :  gesture[6] = nges; gesture[7] = nges; gesture[8] = nges; gesture[9] = nges; gesture[10] = nges; break;
                  default: break;
               }
            }
            break; /* case */
         case 0x82:
            break;
         default:
            break;
      }
   }
}
