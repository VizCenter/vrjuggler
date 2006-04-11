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

namespace gadget
{

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
class Digital
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

      if ((!stable_buffer.empty()) &&
          (stable_buffer.back().size() > (unsigned)devNum))  // If Have entry && devNum in range
      {
         return stable_buffer.back()[devNum];
      }
      else        // No data or request out of range, return default value
      {
         if(stable_buffer.empty())
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


protected:
   SampleBuffer_t    mDigitalSamples; /**< Position samples */
   DigitalData       mDefaultValue;   /**< Default analog value to return */
};

};

#endif   /* _GADGET_DIGITAL_H_ */
