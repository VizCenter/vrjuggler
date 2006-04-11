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

#ifndef _VPR_Barrier_h_
#define _VPR_Barrier_h_

#include <vpr/vprConfig.h>

#if VPR_THREAD_DOMAIN_INCLUDE == VPR_DOMAIN_IRIX_SPROC
#  include <ulocks.h>
#  include <vpr/md/SPROC/Sync/BarrierSGI.h>
#elif (VPR_THREAD_DOMAIN_INCLUDE == VPR_DOMAIN_NSPR) || \
      (VPR_THREAD_DOMAIN_INCLUDE == VPR_DOMAIN_POSIX)
#  include <vpr/Sync/CondVar.h>
#  include <vpr/Sync/Mutex.h>
#  include <vpr/Sync/Guard.h>


namespace vpr
{

/**
 * Helper class for vpr::Barrier.
 */
class SubBarrier
{
public:
   /**
    * Initialization.
    */
   SubBarrier (int count, Mutex* lock)
      : barrierFinished(lock), runningThreads(count)
   {
   }

   CondVar barrierFinished; //! True if this generation of the barrier is done

   int runningThreads;  //! Number of threads that are still running
};

/**
 * Implements "barrier synchronization" primitive.
 *
 */
class Barrier
{
public:
   /**
    * Constructor for barrier.
    * Creates a barrier that will wait for "count" num threads to synchronize.
    */
   Barrier (unsigned count);

   /** Destructor. */
   ~Barrier();

   /**
    * Blocks the caller until all <count> threads have called <wait> and
    * then allows all the caller threads to continue in parallel.
    */
   bool wait(void);

   /**
    * Tells the barrier to increase the count of the number of threads to
    * syncronize.
    */
   void addProcess();

   /**
    * Tells the barrier to decrease the count of the number of threads to
    * syncronize.
    */
   void removeProcess();

private:
   vpr::CondVar   mCond;   /**< Condition variable and mutex for the barrier. */
   unsigned       mThreshold;
   unsigned       mCount;
   unsigned       mGeneration;

   // = Prevent assignment and initialization.
   void operator= (const Barrier &)
   {;}

   Barrier (const Barrier &)
   {;}
};

} // End of vpr namespace

#endif  /* ! VPR_USE_IRIX_SPROC */

#endif  /* _VPR_Barrier_h_ */
