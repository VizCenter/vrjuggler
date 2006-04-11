/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998, 1999, 2000 by Iowa State University
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

/////////////////////////////////////////////////////////////////////////
//
// positional tracker base class
//
////////////////////////////////////////////////////////////////////////

#ifndef _GADGET_DIGITAL_H_
#define _GADGET_DIGITAL_H_

#include <gadget/gadgetConfig.h>
#include <jccl/Config/ConfigChunkPtr.h>
#include <gadget/Type/DigitalData.h>
#include <gadget/Type/SampleBuffer.h>
#include <vpr/Util/Debug.h>
//#include <vpr/IO/ObjectReader.h>
//#include <vpr/IO/ObjectWriter.h>
#include <gadget/RemoteInputManager/SerializableDevice.h>

namespace gadget
{

   const unsigned short MSG_DATA_DIGITAL = 420;

   /** Digital is the abstract base class that devices with digital data derive from.
   *
   *  Digital is the base class that digital devices must derive from.
   *  Digital inherits from Input, so it has pure virtual function
   *  constraints from Input in the following functions: StartSampling,
   *  StopSampling, Sample, and UpdateData. <br>
   *  Digital adds one new pure virtual function, GetDigitalData for
   *  retreiving the digital data, similar to the addition for Position and
   *  Analog.
   *
   * @see Input
   */
   class Digital : public SerializableDevice
   {
   public:
      typedef gadget::SampleBuffer<DigitalData> SampleBuffer_t;

   public:
      /**
       * Enum for the state of the digital buttons.
       * Used in DigitalProxy.
       */
      enum State
      {
         OFF=0, ON=1, TOGGLE_ON=2, TOGGLE_OFF=3
      };

   public:
      /* Constructor/Destructors */
      Digital()
      {
         //vprDEBUG(vprDBG_ALL,4)<<"*** Digital::Digital()\n"<< vprDEBUG_FLUSH;
      }

      virtual ~Digital()
      {
      }

      virtual bool config(jccl::ConfigChunkPtr c)
      {
         //vprDEBUG(vprDBG_ALL,4)<<"*** Digital::config()\n"<< vprDEBUG_FLUSH;
         return true;
      }

      /**
       * Gets the digital data for the given devNum.
       *
       * @return Digital 0 or 1, if devNum makes sense.
       *         -1 is returned if function fails or if devNum is out of range.
       * @note If devNum is out of range, function will fail, possibly issuing
       *       an error to a log or console - but will not ASSERT.
       */
      const DigitalData getDigitalData(int devNum = 0)
      {
         SampleBuffer_t::buffer_t& stable_buffer = mDigitalSamples.stableBuffer();

         if ( (!stable_buffer.empty()) &&
              (stable_buffer.back().size() > (unsigned)devNum) )  // If Have entry && devNum in range
         {
            return stable_buffer.back()[devNum];
         }
         else        // No data or request out of range, return default value
         {
            if ( stable_buffer.empty() )
            {
               vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL) << "Warning: Digital::getDigitalData: Stable buffer is empty. If this is not the first read, then this is a problem.\n" << vprDEBUG_FLUSH;
            }
            else
            {
               vprDEBUG(vprDBG_ALL, vprDBG_CONFIG_LVL) << "Warning: Digital::getDigitalData: Requested devNum " << devNum << " is not in the range available.  May have configuration error\n" << vprDEBUG_FLUSH;
            }
            return mDefaultValue;
         }
      }

      const SampleBuffer_t::buffer_t& getDigitalDataBuffer()
      {
         return mDigitalSamples.stableBuffer();
      }
      virtual std::string getBaseType()
      {
         return std::string("Digital");
      }

      virtual vpr::ReturnStatus writeObject(vpr::ObjectWriter* writer)
      {
         //std::cout << "[Remote Input Manager] In Digital write" << std::endl;
         SampleBuffer_t::buffer_t& stable_buffer = mDigitalSamples.stableBuffer();
         writer->writeUint16(MSG_DATA_DIGITAL);                               // Write out the data type so that we can assert if reading in wrong place
         if ( !stable_buffer.empty() )
         {
            mDigitalSamples.lock();
            writer->writeUint16(stable_buffer.size());                                 // Write the # of vectors in the stable buffer
            //std::cout << "Digital Buffer size: " << stable_buffer.size() << std::endl;
            for ( unsigned j=0;j<stable_buffer.size();j++ )                               // For each vector in the stable buffer
            {
               writer->writeUint16(stable_buffer[j].size());                           // Write the # of DigitalDatas in the vector
               //std::cout << "Digital Data Size: "
               //<< stable_buffer.back().size() << std::endl;
               //std::cout << "ME: ";
               for ( unsigned i=0;i<stable_buffer[j].size();i++ )                         // For each DigitalData in the vector
               {
                  writer->writeUint32((vpr::Uint32)stable_buffer[j][i].getDigital());  // Write Digital Data(int)
                  //std::cout << (vpr::Uint32)stable_buffer[j][i].getDigital();
                  writer->writeUint64(stable_buffer[j][i].getTime().usec());           // Write Time Stamp vpr::Uint64
               }
               //std::cout << std::endl;
            }
            mDigitalSamples.unlock();
         }
         else        // No data or request out of range, return default value
         {
            writer->writeUint16(0);
         }
         return vpr::ReturnStatus::Succeed;
      }

      virtual vpr::ReturnStatus readObject(vpr::ObjectReader* reader, vpr::Uint64* delta)
      {
            //std::cout << "[Remote Input Manager] In Digital read" << std::endl;

            // ASSERT if this data is really not Digital Data
         vpr::Uint16 temp = reader->readUint16();
         vprASSERT(temp==MSG_DATA_DIGITAL && "[Remote Input Manager]Not Digital Data");

         std::vector<DigitalData> dataSample;

         unsigned numDigitalDatas;
         vpr::Uint32 value;
         vpr::Uint64 timeStamp;
         DigitalData temp_digital_data;

         unsigned numVectors = reader->readUint16();
         //std::cout << "Stable Digital Buffer Size: "  << numVectors << std::endl;
         mDigitalSamples.lock();
         for ( unsigned i=0;i<numVectors;i++ )
         {
            numDigitalDatas = reader->readUint16();
            //std::cout << "Digital Data Size: "    << numDigitalDatas << std::endl;
            dataSample.clear();
            //std::cout << "ME: ";
            for ( unsigned j=0;j<numDigitalDatas;j++ )
            {
               value = reader->readUint32();       //Write Digital Data(int)
               //std::cout << value;
               timeStamp = reader->readUint64();                  //Write Time Stamp vpr::Uint64
               temp_digital_data.setDigital(value);
               temp_digital_data.setTime(vpr::Interval(timeStamp + *delta,vpr::Interval::Usec));
               dataSample.push_back(temp_digital_data);
            }
            //std::cout << std::endl;

            mDigitalSamples.addSample(dataSample);

         }
         mDigitalSamples.unlock();
         mDigitalSamples.swapBuffers();
         return vpr::ReturnStatus::Succeed;
      }


   protected:
      SampleBuffer_t    mDigitalSamples; /**< Position samples */
      DigitalData       mDefaultValue;   /**< Default analog value to return */
   };

} // End of gadget namespace

#endif   /* _GADGET_DIGITAL_H_ */
