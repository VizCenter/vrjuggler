/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998, 1999, 2000, 2001, 2002
 *   by Iowa State University
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
// Standaolne bird tracking class
//
// Note: untested with a standalone configuration, probably doesn't work
////////////////////////////////////////////////////////////////////////
#ifndef _VJ_BIRD_H_
#define _VJ_BIRD_H_

#include <vjConfig.h>
#include <Input/vjPosition/vjPosition.h>
#include <Threads/vjThread.h>
#include <Input/vjPosition/vjFlock.h>

//: A start at a standalone bird derice driver.
//
//  vjBird is based off of vjFlock, but for one bird only, the
//  group mode things have been stripped, and should be close to
//  a working implementation, but has not been tested a standalone
//  bird.
class vjBird : public vjPosition
{
public:
   /** @name Construction/Destruction */
   //@{
   vjBird();
   ~vjBird();
   //@}

   virtual bool config(vjConfigChunk* c);

   /** @name vjInput pure virtual functions
    *
    *  pure virtual functions required from vjInput
    */
   //@{
   int startSampling();
   int stopSampling();
   int sample();
   void updateData();
   //@}

   /** @name vjInput virtual functions
    *
    *  virtual functions that inherited members should
    *  override but are not required to
    */
   //@{
   char* getDeviceName() { return "vjBird";}
   //@}

   /** @name vjPosition pure virtual functions
    *
    *  pure virtual functions required by vjPosition
    */
   //@{
   //old function
   //void getData(vjPOS_DATA* &data);
   // XXX: Bad things live here
   vjMatrix* getPosData(int dev = 0); // 0 base
   //@}


   /** @name vjBird settings functions
    *
    *  Functions for chaning the Bird settings, set functions can only be
    *  called when the device is not in sampling mode.  These functions
    *  shouldn't be used by apps using configchunks.
    */
   //@{
   void setSync(int sync);
   int  getSync()
   {return syncStyle;}

   void setBlocking(int blVal);
   int  getBlocking()
   { return blocking;}

   void setFilters(BIRD_FILT f);
   int  getFilters()
   { return filter;}

   void setHemisphere(BIRD_HEMI h);
   int  getHemisphere()
   {return hemisphere;}

   void setReportRate(char rRate);
   char getReportRate()
   {return repRate;}
   //@}

private:
   void positionCorrect(float&x,float&y,float&z);
   void initCorrectionTable();
   vjThread*   myThread;      // ptr to our thread object
   CalStruct   caltable;
   int  syncStyle, blocking;
   char repRate;
   BIRD_HEMI hemisphere;
   BIRD_FILT filter;
};

#endif
