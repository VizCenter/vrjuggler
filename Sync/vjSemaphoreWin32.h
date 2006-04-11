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


#ifndef _vjSemaphoreWin32_h_
#define _vjSemaphoreWin32_h_
//----------------------------------------------
// vjSemaphoreWin32
//
// Purpose:
//:    Semaphore wrapper for the Win32 systems
//
//
// Author:
//	Andy Himberger
//
// Date: 11-9-97
//-----------------------------------------------
#include <windows.h>
#include <SharedMem/vjMemPool.h>
#include <SharedMem/vjMemPoolWin32.h>
#include <Kernel/vjDebug.h>
#include <Threads/vjThread.h>

//!PUBLIC_API:
class vjSemaphoreWin32
{
public:
   vjSemaphoreWin32 (int initialValue = 1)
   {
//      // BUG:	
//      if (semaphorePool == NULL)
//      {
//         semaphorePool = new vjMemPoolWin32(65536, 32, "memSemaphorePoolWin32XXXXXX");
//         attachedCounter = static_cast<int*>(semaphorePool->allocate(sizeof(int)));
//         *attachedCounter = 0;
//      }
//      *attachedCounter = *attachedCounter + 1;      // Track how many semaphores are allocated

      //DebugLock.acquire();
      //std::cerr << vjThread::self() << " vjSemaphoreWin32::vjSemaphoreWin32: attachedCounter: " << *attachedCounter << std::endl;
      //DebugLock.release();

      // ----- Allocate the semaphore ----- //
      sema = CreateSemaphore(NULL,initialValue,99,NULL);
   }

   ~vjSemaphoreWin32(void)
   {
      // ---- Delete the semaphore --- //
      CloseHandle(sema);

      // ---- Deal with the pool --- //
//      *attachedCounter = *attachedCounter - 1;     // Track how many Semaphore are allocated

      //DebugLock.acquire();
      //std::cerr << vjThread::self() << " vjSemaphoreWin32::~vjSemaphoreWin32: attachedCounter: " << *attachedCounter << std::endl;
      //DebugLock.release();

//      if (*attachedCounter == 0)
//      {
//         semaphorePool->deallocate(attachedCounter);
//         attachedCounter = NULL;
//         delete semaphorePool;
//         semaphorePool = NULL;
//      }

   }

   //---------------------------------------------------------
   //: Lock the semaphore.
   //
   //! RETURNS:  1 - Acquired
   //! RETURNS: -1 - Error
   //---------------------------------------------------------
   int acquire() const
   {
      DWORD dw = WaitForSingleObject(sema,INFINITE);
      if (dw == WAIT_OBJECT_0)
      {
         return 1;
      }
      return -1;
   }

   //----------------------------------------------------------
   //: Acquire a read semaphore.
   //----------------------------------------------------------
   int acquireRead() const
   {
      return this->acquire();     // No special "read" semaphore -- For now
   }

   //----------------------------------------------------------
   //: Acquire a write semaphore.
   //----------------------------------------------------------
   int acquireWrite() const
   {
      return this->acquire();     // No special "write" semaphore -- For now
   }

   //---------------------------------------------------------
   //: Try to acquire the semaphore immediately.  Does not
   //+ block.
   //
   //! RETURNS: 1 - Acquired
   //! RETURNS: 0 - Not acquired
   //---------------------------------------------------------
   int tryAcquire () const
   {
      DWORD dw = WaitForSingleObject(sema,0);
      if (dw == WAIT_OBJECT_0)
      {
         return 1;
      }
      return 0;
   }

   //----------------------------------------------------------
   //:  Try to acquire a read semaphore.
   //----------------------------------------------------------
   int tryAcquireRead () const
   {
      return this->tryAcquire();
   }

   //----------------------------------------------------------
   //:  Try to acquire a write semaphore.
   //----------------------------------------------------------
   int tryAcquireWrite () const
   {
      return this->tryAcquire();
   }

   //---------------------------------------------------------
   //: Release the semaphore.
   //
   //! RETURNS:  0 - Success
   //! RETURNS: -1 - Error
   //---------------------------------------------------------
   int release() const
   {
      return ReleaseSemaphore(sema,1,NULL);
   }

   //---------------------------------------------------------
   //: Reset the semaphore.
   //
   //! RETURNS:  0 - Success
   //! RETURNS: -1 - Error
   //
   //! NOTE: If processes are waiting on the semaphore, the
   //+ results are undefined.
   //---------------------------------------------------------
   int reset(int val)
   {
      if (!CloseHandle(sema)) return -1;
      sema = CreateSemaphore(NULL,val,99,NULL);
      if (sema == NULL) return -1;
      return 0;
   }

   //---------------------------------------------------------
   //: Dump the semaphore debug stuff and current state.
   //---------------------------------------------------------
   void dump (FILE* dest = stderr, const char* message = "\n------ Semaphore Dump -----\n") const
   {
       std::cout << "vjSemaphoreWin32::dump() \nNot implemented on Win32"
                 << std::endl;
   }

protected:
   HANDLE sema;
   int    initVal;

   // = Prevent assignment and initialization.
   void operator= (const vjSemaphoreWin32 &) {}
   vjSemaphoreWin32 (const vjSemaphoreWin32 &) {}

//   // Problem here.  Fork will not like these.
//   static vjMemPoolWin32* semaphorePool;
//   static int* attachedCounter;
};

//vjMemPoolWin32* vjSemaphoreWin32::semaphorePool = NULL;
//int* vjSemaphoreWin32::attachedCounter = NULL;

#endif
