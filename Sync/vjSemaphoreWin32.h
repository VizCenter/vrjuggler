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

class vjSemaphoreWin32
{
public:
   vjSemaphoreWin32 (int initialValue = 1)
   {
      // BUG:	
      if (semaphorePool == NULL)
      {
         semaphorePool = new vjMemPoolWin32(65536, 32, "memSemaphorePoolWin32XXXXXX");
         attachedCounter = static_cast<int*>(semaphorePool->allocate(sizeof(int)));
         *attachedCounter = 0; 
      }
      *attachedCounter = *attachedCounter + 1;      // Track how many semaphores are allocated

      DebugLock.acquire();
      cerr << vjThread::self() << " vjSemaphoreWin32::vjSemaphoreWin32: attachedCounter: " << *attachedCounter << endl;
      DebugLock.release();

      // ----- Allocate the semaphore ----- //
      sema = CreateSemaphore(NULL,initialValue,99,NULL);
   }

   ~vjSemaphoreWin32(void)
   {
      // ---- Delete the semaphore --- //
      CloseHandle(sema);

      // ---- Deal with the pool --- //
      *attachedCounter = *attachedCounter - 1;     // Track how many Semaphore are allocated  

      DebugLock.acquire();
      cerr << vjThread::self() << " vjSemaphoreWin32::~vjSemaphoreWin32: attachedCounter: " << *attachedCounter << endl;
      DebugLock.release();

      if (*attachedCounter == 0)
      {
         semaphorePool->deallocate(attachedCounter);
         attachedCounter = NULL;
         delete semaphorePool;
         semaphorePool = NULL;
      }

   }

   //---------------------------------------------------------
   // int aquire()
   //
   // PURPOSE:
   //:   Lock the Semaphore
   //! RETURNS:
   //   1 - Aquired
   //  -1 - Error
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
//:  Acquire a read Semaphore
//----------------------------------------------------------
   int acquireRead() const
   {
      return this->acquire();     // No special "read" semaphore -- For now
   }

//----------------------------------------------------------
//:  Acquire a write Semaphore
//----------------------------------------------------------
   int acquireWrite() const
   {
      return this->acquire();     // No special "write" semaphore -- For now
   }

//---------------------------------------------------------
// int tryAcquire()
//
//! PURPOSE:
//   Try to acquire the semaphore immediately.  Does not block.
//! RETURNS:
//   1 - Aquired
//   0 - Not Aquired
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
//:  Try to aquire a read Semaphore
//----------------------------------------------------------
   int tryAcquireRead () const
   {
      return this->tryAcquire();
   }

//----------------------------------------------------------
//:  Try to aquire a write Semaphore
//----------------------------------------------------------
   int tryAcquireWrite () const
   {
      return this->tryAcquire();
   }

//---------------------------------------------------------
// int release()
//
// PURPOSE:
//   Release the Semaphore. 
//! RETURNS:
//   0 - Success
//  -1 - Error
//---------------------------------------------------------
   int release() const
   {
      return ReleaseSemaphore(sema,1,NULL);   
   }

//---------------------------------------------------------
// int reset(int val)
//
// PURPOSE:
//:  Reset the Semaphore. 
//! RETURNS:
//   0 - Success
//  -1 - Error
//! NOTE:
//   If processes are waiting on the semaphore, the results are undefined
//---------------------------------------------------------
   int reset(int val)
   {
      if (!CloseHandle(sema)) return -1;
      sema = CreateSemaphore(NULL,val,99,NULL);
      if (sema == NULL) return -1;
      return 0;
   }

//---------------------------------------------------------
// void dump()
//
// PURPOSE:
//:  Dump the semaphore debug stuff and current state.
//---------------------------------------------------------
   void dump (FILE* dest = stderr, const char* message = "\n------ Semaphore Dump -----\n") const
   {
      cout << "vjSemaphoreWin32::dump() " << endl; 
   }


protected:
   HANDLE sema;
   int    initVal;

   // = Prevent assignment and initialization.
   void operator= (const vjSemaphoreWin32 &) {}
   vjSemaphoreWin32 (const vjSemaphoreWin32 &) {}  

   // Problem here.  Fork will not like these.
   static vjMemPoolWin32* semaphorePool;
   static int* attachedCounter;
};

vjMemPoolWin32* vjSemaphoreWin32::semaphorePool = NULL;
int* vjSemaphoreWin32::attachedCounter = NULL;

#endif
