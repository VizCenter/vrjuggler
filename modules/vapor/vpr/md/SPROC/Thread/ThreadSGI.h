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
 *************** <auto-copyright.pl END do not edit this line> ***************/

#ifndef _VPR_THREAD_SGI_H_
#define _VPR_THREAD_SGI_H_
//#pragma once

// NOTE: This file(vprThreadSGI.h) MUST be included by vprThread.h.
// Not the other way around

#include <vpr/vprConfig.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <signal.h>
#include <sched.h>
#include <unistd.h>

#include <vpr/vprTypes.h>
#include <vpr/Thread/ThreadFunctor.h>
#include <vpr/Thread/BaseThread.h>


namespace vpr
{

/**
 * Threads implementation using multiple processes created with sproc(2).
 */
class ThreadSGI : public BaseThread
{
public:
   /***** CONSTRUCTORS ******/

   /**
    * Spawning constructor.
    * This will actually start a new thread that will execute the specified
    * function.
    */
   ThreadSGI(thread_func_t func, void* arg = 0,
             BaseThread::VPRThreadPriority priority = VPR_PRIORITY_NORMAL,
             BaseThread::VPRThreadScope scope = VPR_LOCAL_THREAD,
             BaseThread::VPRThreadState state = VPR_JOINABLE_THREAD,
             size_t stack_size = 0);

   /**
    * Spawning constructor with arguments (functor version).
    * This will start a new thread that will execute the specified
    * function.
    */
   ThreadSGI(BaseThreadFunctor* functorPtr,
             BaseThread::VPRThreadPriority priority = VPR_PRIORITY_NORMAL,
             BaseThread::VPRThreadScope scope = VPR_LOCAL_THREAD,
             BaseThread::VPRThreadState state = VPR_JOINABLE_THREAD,
             size_t stack_size = 0);

   virtual ~ThreadSGI()
   {;}

   /**
    * Spawns a new thread that will execute functorPtr.
    *
    * @pre None.
    * @post A thread (with any specified attributes) is created that begins
    *        executing func().  Depending on the scheduler, it may being
    *        execution immediately, or it may block for a short time before
    *        beginning execution.
    *
    * @param functorPtr  Function to be executed by the thread.
    * @param flags  Flags for the thread--not currently used in this
    *                implementation (optional).
    *
    * @return A non-zero value is returned upon successful thread creation.
    *         -1 is returned if an error occurred.
    */
   virtual int spawn(BaseThreadFunctor* functorPtr,
                     BaseThread::VPRThreadPriority priority = VPR_PRIORITY_NORMAL,
                     BaseThread::VPRThreadScope scope = VPR_LOCAL_THREAD,
                     BaseThread::VPRThreadState state = VPR_JOINABLE_THREAD,
                     size_t stack_size = 0)
   {
      mThreadPID = sproc(thread_func_t(&vprThreadFunctorFunction),
                         PR_SADDR | PR_SFDS, functorPtr);
      return mThreadPID;
   }

   /**
    * Called by the spawn routine to start the user thread function.
    *
    * @pre Called ONLY by a new thread
    * @post The new thread will have started the user thread function
    */
   void startThread(void* null_param);

private:
   /**
    * The functor to call from startThread
    */
   BaseThreadFunctor* mUserThreadFunctor;

public:

   /**
    * Makes the calling thread wait for the termination of the specified
    * thread.
    *
    * @return 0 is returned on successful completion.  -1 is returned on
    *         failure.
    */
   virtual int join(void** = 0);

   /**
    * Resumes the execution of a thread that was previously suspended using
    * suspend().
    *
    * @pre The specified thread was previously suspended using the
    *       suspend() member function.
    * @post The specified thread is sent the SIGCONT signal and is allowed
    *        to begin executing again.
    *
    * @return 0 is returned on successful completion.  -1 is returned on
    *         failure.
    */
   virtual int resume()
   {
      return ::kill(mThreadPID, SIGCONT);
   }

   /**
    * Suspends the execution of this thread.
    *
    * @pre None.
    * @post This thread is sent the SIGSTOP signal and is thus suspended
    *        from execution until the member function resume() is called.
    *
    * @return 0 is returned on successful completion.  -1 is returned on
    *         failure.
    */
   virtual int suspend()
   {
      return ::kill(mThreadPID, SIGSTOP);
   }

   /**
    * Gets this thread's priority.
    *
    * @pre None.
    * @post The priority of this thread is returned in the integer pointer
    *        variable.
    *
    * @param prio  Pointer to an int variable that will have the thread's
    *               priority stored in it.
    *
    * @return 0 is returned on successful completion.  -1 is returned on
    *         failure.
    */
   virtual int getprio(int* prio)
   {
      *prio = getpriority(PRIO_PROCESS, mThreadPID);

      if ((*prio)== -1)
      {
         return -1;
      }
      else
      {
         return 0;
      }
   }

   /**
    * Sets this thread's priority.
    *
    * @pre None.
    * @post This thread has its priority set to the specified value.
    *
    * @param prio  The new priority of the specified thread.
    *
    * @return 0 is returned on successful completion.  -1 is returned on
    *         failure.
    */
   int setprio(int prio)
   {
      return setpriority(PRIO_PROCESS, mThreadPID, prio);
   }

   /**
    * Yields execution of the calling thread to allow a different blocked
    * thread to execute.
    *
    * @pre None.
    * @post The caller yields its execution control to another thread or
    *        process.
    */
   static void yield()
   {
      sginap(0);
   }

   /**
    * Causes the calling thread to sleep for the given number of microseconds.
    *
    * @param micro The number of microseconds to sleep.
    */
   static int usleep(vpr::Uint32 micro)
   {
       return ::usleep(micro);
   }

   /**
    * Causes the calling thread to sleep for the given number of milliseconds.
    *
    * @param milli The number of milliseconds to sleep.
    */
   static int msleep(vpr::Uint32 milli)
   {
      // usleep() cannot sleep for more than 1 second, so we have to work
      // around that here.  First, we sleep for N seconds.
      if ( milli >= 1000 )
      {
         ThreadSGI::sleep(milli / 1000);
      }

      // Then we finish off by sleeping for (N mod 1000) milliseconds.
      return ThreadSGI::usleep((milli % 1000) * 1000);
   }

   /**
    * Causes the calling thread to sleep for the given number of seconds.
    *
    * @param seconds The number of seconds to sleep.
    */
   static int sleep(vpr::Uint32 seconds)
   {
       return ::sleep(seconds);
   }

   /**
    * Sends the specified signal to this thread (not necessarily SIGKILL).
    *
    * @pre None.
    * @post This thread receives the specified signal.
    *
    * @param signum  The signal to send to the specified thread.
    *
    * @return 0 is returned on successful completion.  -1 is returned on
    *         failure.
    */
   virtual int kill(int signum)
   {
      return ::kill(mThreadPID, signum);
   }

   /**
    * Kills (cancels) this thread.
    *
    * @pre None.
    * @post This thread is cancelled.  Depending on the cancellation
    *        attributes of the specified thread, it may terminate
    *        immediately, it may wait until a pre-defined cancel point to
    *        stop or it may ignore the cancel altogether.  Thus, immediate
    *        cancellation is not guaranteed.
    *
    * @note For the sake of clarity, it is probably better to use the
    *        cancel() routine instead of kill() because a two-argument
    *        version of kill() is also used for sending signals to threads.
    */
   virtual void kill()
   {
      kill(SIGKILL);
   }

   /**
    * Outputs the state of the object.
    */
   std::ostream& outStream(std::ostream& out)
   {
      out.setf(std::ios::right);
      out << std::setw(7) << std::setfill('0') << mThreadPID << "/";
      out.unsetf(std::ios::right);
      BaseThread::outStream(out);
      out << std::setfill(' ');
      return out;
   }

private:
   pid_t mThreadPID;      /**<  pid_t data structure for this thread */

   // --------  STATICS ------ //
   // This data is used to maintain a thread table in the system
   // It is here because the self() function needs to use system specific
   // information.
public:
   /**
    * Gets a pointer to the thread we are in.
    *
    * @return NULL - Thread is not in global table
    * @return NonNull - Ptr to the thread that we are running within
    */
   static BaseThread* self()
   {
      return getLocalThreadPtr();
   }

private:
   /**
    * Sets the thread ptr stored in the local PRDA area.
    *
    * @note PRDA is a memory address that each thread has a seperate copy of.
    */
   static void setLocalThreadPtr(ThreadSGI* threadPtr)
   {
      ((ThreadInfo*)PRDA->usr_prda.fill)->mThreadPtr = threadPtr;
   }

   /**
    * Gets the thread ptr stored in the local PRDA area.
    */
   static ThreadSGI* getLocalThreadPtr()
   {
      return ((ThreadInfo*)PRDA->usr_prda.fill)->mThreadPtr;
   }

   /**
    * Structure for storing thread specific information.
    */
   struct ThreadInfo
   {
      ThreadSGI* mThreadPtr;
   };
};

} // End of vpr namespace


#endif   /* _VPR_THREAD_SGI_H_ */
