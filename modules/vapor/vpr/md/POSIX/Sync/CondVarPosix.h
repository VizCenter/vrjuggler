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

/*
 * ----------------------------------------------------------------------------
 * NOTES:
 *    - This file (CondVarPosix.h) must be included by vpr/Sync/Cond.h, not
 *      the other way around.
 * ----------------------------------------------------------------------------
 */

#ifndef _VPR_COND_VAR_POSIX_H_
#define _VPR_COND_VAR_POSIX_H_

#include <vpr/vprConfig.h>
#include <pthread.h>
#include <vpr/md/POSIX/Sync/MutexPosix.h>
#include <vpr/Util/Interval.h>
#include <sys/time.h>


namespace vpr
{

/**
 * Condition variable wrapper for POSIX-compliant systems using pthreads
 * condition variables for the implementation.
 */
class CondVarPosix
{
public:
   /**
    * Constructor for vpr::CondVarPosix class.
    *
    * @pre None.
    * @post The condition variable is intialized, and the mutex variable
    *       associated with it is defined.  These two steps must be done
    *       before any other member functions can use them.
    *
    * @param mutex Pointer to a vpr::MutexPosix variable that is used in
    *              association with this condition variable in this class
    *              (optional).
    */
   CondVarPosix (MutexPosix* mutex = NULL)
   {
      // Initialize the condition variable.
      pthread_cond_init(&mCondVar, NULL);

      // If the caller did not specify a mutex variable to use with
      // the condition variable, use mDefaultMutex.
      if ( mutex == NULL )
      {
         mutex = &mDefaultMutex;
      }

      mCondMutex = mutex;
   }

   /**
    * Destructor for vpr::CondVarPosix class.
    *
    * @pre None.
    * @post The condition variable is destroyed.
    */
   ~CondVarPosix (void)
   {
      pthread_cond_destroy(&mCondVar);
   }

   /**
    * Blocks on a condition.
    *
    * @pre The mutex variable associated with this condition variable must be
    *      locked.
    * @post The condition variable is locked.  If it was previously locked,
    *       the caller blocks until signaled.
    *
    * @return vpr::ReturnStatus::Succeed is returned after blocking.
    *         vpr::ReturnStatus::Fail is returned if an error occurred when
    *         trying to wait on the condition variable.
    */
   vpr::ReturnStatus wait (vpr::Interval time_to_wait = vpr::Interval::NoTimeout)
   {
      // ASSERT:  We have been locked
      vpr::ReturnStatus status;

      // If not locked ...
      if ( mCondMutex->test() == 0 )
      {
         std::cerr << "vpr::CondVarPosix::wait: INCORRECT USAGE: Mutex was not "
                   << "locked when wait invoked!!!\n";

         status.setCode(vpr::ReturnStatus::Fail);
      }
      else
      {
         // The mutex variable must be locked when passed to
         // pthread_cond_wait().
         if ( vpr::Interval::NoTimeout == time_to_wait )
         {
            if ( pthread_cond_wait(&mCondVar, &(mCondMutex->mMutex)) != 0 )
            {
               status.setCode(vpr::ReturnStatus::Fail);
            }
         }
         else
         {
            struct timeval  now;                // The current time
            struct timespec abs_timeout;        // The absolute time of the timeout
            int    retcode;

            // Calculate the absolute time for wait timeout
            gettimeofday(&now, NULL);
            abs_timeout.tv_sec = now.tv_sec + time_to_wait.sec();

            vpr::Uint64 left_over_us = time_to_wait.usec()%1000000;     // Get the left over usecs
            abs_timeout.tv_nsec = 1000 * (now.tv_usec + left_over_us);

            retcode = pthread_cond_timedwait(&mCondVar, &(mCondMutex->mMutex), &abs_timeout);

            if(0 == retcode)
            { status.setCode(vpr::ReturnStatus::Succeed); }
            else if(ETIMEDOUT == retcode)
            { status.setCode(vpr::ReturnStatus::Timeout); }
            else
            { status.setCode(vpr::ReturnStatus::Fail); }
         }
      }

      return status;
   }

   /**
    * Signals a thread waiting on this condition variable.
    *
    * @pre The condition variable must be locked.
    * @post The condition variable is unlocked, and a signal is sent to a
    *       thread waiting on it.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the signal is sent
    *         successfully.  vpr::ReturnStatus::Fail is returned if the
    *         signalling failed.
    */
   vpr::ReturnStatus signal (void)
   {
      // ASSERT:  We have been locked
      if ( pthread_cond_signal(&mCondVar) == 0 )
      {
         return vpr::ReturnStatus(vpr::ReturnStatus::Succeed);
      }
      else
      {
         return vpr::ReturnStatus(vpr::ReturnStatus::Fail);
      }
   }

   /**
    * Signals all waiting threads.
    *
    * @pre The mutex variable associated with this condition variable
    *      should be locked.
    * @post The condition variable is unlocked, and all waiting threads
    *       are signaled of this event.
    *
    * @return vpr::ReturnStatus::Succeed is returned if a signal was broadcast
    *         to all waiting threads successfully.
    *         vpr::ReturnStatus::Fail is returned to indicate an error with the
    *         signal broadcast.
    */
   vpr::ReturnStatus broadcast (void)
   {
      // ASSERT:  We have been locked

      // If not locked ...
      if ( mCondMutex->test() == 0 )
      {
         std::cerr << "vpr::CondVarPosix::broadcast: Mutex was not locked when "
                   << "broadcast called!!!\n";
      }

      if ( pthread_cond_broadcast(&mCondVar) == 0 )
      {
         return vpr::ReturnStatus(vpr::ReturnStatus::Succeed);
      }
      else
      {
         return vpr::ReturnStatus(vpr::ReturnStatus::Fail);
      }
   }

   /**
    * Acquires a lock on the mutex variable associated with this condition
    * variable.
    *
    * @pre None.
    * @post A lock is acquired on the mutex variable associated with the
    *      condition variable.  If a lock is acquired, the caller controls
    *      the mutex variable.  If it was previously locked, the caller
    *      blocks until it is unlocked.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the lock was acquired
    *         successfully. vpr::ReturnStatus::Fail is returned otherwise.
    */
   vpr::ReturnStatus acquire (void)
   {
      return mCondMutex->acquire();
   }

   /**
    * Try to acquire a lock on the mutex variable associated with this
    * condition variable.
    *
    * @pre None.
    * @post If the mutex variable is not already locked, the caller
    *       obtains a lock on it.  If it is already locked, the routine
    *       returns immediately to the caller.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the lock was acquired
    *         successfully.  vpr::ReturnStatus::Fail is returned if the lock
    *         could not be acquired.
    */
   vpr::ReturnStatus tryAcquire (void)
   {
      return mCondMutex->tryAcquire();
   }

   /**
    * Releases the lock on the mutex variable associated with this condition
    * variable.
    *
    * @pre None.
    * @post The lock held by the caller on the mutex variable is released.
    */
   vpr::ReturnStatus release (void)
   {
      return mCondMutex->release();
   }

   /**
    * Changes the condition variable mutex to be the specifiec mutex
    * variable.
    *
    * @pre The specified mutex variable must be initialized.
    * @post The condition variable associated with the mutex variable is
    *       reset to the specified variable.
    *
    * @param mutex Pointer to a vpr::MutexPosix variable that is used in
    *              association with the condition variable in this class.
    *
    * @note NEVER call except to initialize explicitly.
    */
   void setMutex (vpr::MutexPosix* mutex)
   {
      // NOT exactly correct, but just make sure not to leave it locked
      mutex->release();
      mCondMutex = mutex;
   }

   /** Test the mutex to see if it is held.
    */
   int test()
   { return mCondMutex->test(); }

   /**
    * Prints out information about the condition variable to stderr.
    *
    * @pre None.
    * @post All important data and debugging information related to the
    *       condition variable and its mutex are dumped to stderr.
    */
   void dump (void) const
   {
      std::cerr << "------------- vpr::CondVarPosix::Dump ---------\n"
                << "Not Implemented yet.\n";
   }

private:
   pthread_cond_t mCondVar;      /**< Condition variable */
   MutexPosix*    mCondMutex;    /**< Mutex for the condition variable */
   MutexPosix     mDefaultMutex; /**< A default mutex variable */

   // = Prevent assignment and initialization.
   void operator= (const CondVarPosix&) {;}
   CondVarPosix (const CondVarPosix &c) {;}
};

}; // End of vpr namespace


#endif  /* _VPR_COND_POSIX_H_ */
