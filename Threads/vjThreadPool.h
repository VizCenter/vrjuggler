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


#ifndef _VJTHREAD_POOL_H
#define _VJTHREAD_POOL_H


#include <vjConfig.h>

#include <Threads/vjThread.h>
#include <SharedMem/vjMemory.h>
#include <Sync/vjSemaphore.h>


//: Helper class for vjThreadPool
class vjOneThread
{
public:
    vjOneThread() : thread(NULL), next(NULL), functor(NULL), threadWait(0) {
        next = NULL;
    }

public:
    vjThread* thread;               //: Handle of the thread process
    vjOneThread* next;              //: -> next vjOneThread ready to run
    vjBaseThreadFunctor* functor;   //: -> function thread is to call
    vjSemaphore threadWait;         //: thread waits for work here
};

std::ostream& operator<< (std::ostream&, vjOneThread&);


//----------------------------------------------
//: A pool of threads to process user jobs
//
//
// Outline of thread pool <br>
//   |-Need to initialize all thread given a parameter to the constructor of the # to use <br>
//   |-Need a function to block until all work is done that has started <br>
//   |-Just repetiviely add work to the process <br>
//
// Author:
// Allen Bierbaum
//
// Date: 2-5-97
//-----------------------------------------------
//!PUBLIC_API:
class vjThreadPool
{
public:
    // -----------------------------------------------------------------------
    //: Constructor.
    // -----------------------------------------------------------------------
    vjThreadPool(int numToStartWith = 1);

    // -----------------------------------------------------------------------
    //: Give a function to the processes.  Start a function going
    //+ asynchronously.  Called by master process.
    // -----------------------------------------------------------------------
    void startFunc (vj_thread_func_t func, void* arg = NULL)
    {
        vjThreadNonMemberFunctor* NonMemFunctor = new vjThreadNonMemberFunctor(func, arg);
        this->startFunc(NonMemFunctor);
    }

    // -----------------------------------------------------------------------
    // -----------------------------------------------------------------------
    void startFunc( vjBaseThreadFunctor* theFunctor, void* argument)
    {
        theFunctor->setArg(argument);
        this->startFunc(theFunctor);
    }

    // -----------------------------------------------------------------------
    // -----------------------------------------------------------------------
    void startFunc( vjBaseThreadFunctor* theFunctor)
    {
        vjOneThread* theThread = getThread();

        theThread->functor = theFunctor;     /* set address of func to exec */
        theThread->threadWait.release();     /* wake up sleeping process */
        //delete theFunctor;
    }

    // -----------------------------------------------------------------------
    // -----------------------------------------------------------------------
    vjOneThread* addThread(void);

    // -----------------------------------------------------------------------
    //: Body of a general-purpose child process. The argument, which must be
    //+ declared void* to match the func prototype, is the vjOneThread
    //+ structure that represents this process.   The contents of that
    //+ struct, in particular threadWait, MUST be initialized by the parent.
    // -----------------------------------------------------------------------
    void threadLoop(void* theThreadAsVoid);

    // -----------------------------------------------------------------------
    //: Wait for work to do.  Put a vjOneThread structure on the ready list
    //+ and sleep on it.  Called by a child process when its work is done.
    // -----------------------------------------------------------------------
    void threadSleep(vjOneThread* theThread);

    // -----------------------------------------------------------------------
    //: Get a process to run.  Acquire a vjOneThread structure from the ready
    //+ list, waiting if necessary.  Called by the master process as part of
    //+ dispatching a thread.
    // -----------------------------------------------------------------------
    vjOneThread* getThread(void);

    // -----------------------------------------------------------------------
    //: Waits until all threads are done doing their work.
    // -----------------------------------------------------------------------
    void
    barrier (void) {
        finishedLock.acquire();      // Get the lock that means threads done
        finishedLock.release();      // Reset it to done
    }

    // -----------------------------------------------------------------------
    // -----------------------------------------------------------------------
    void printList(void);

private:
    vjSemaphore readyThreads; //: count represents threads ready to work
    vjMutex listLock;      //: mutex control of threadList head
    vjMutex workingCountLock; //: mutex on thread count
    vjMutex finishedLock;  //: Lock for wether thread are finished, lock -> doing work
    vjOneThread* listHead; //: -> first ready vjOneThread
    volatile int workingCount;   //: Number of threads currently doing work
};

#endif
