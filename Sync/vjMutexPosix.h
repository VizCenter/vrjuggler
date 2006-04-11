/*
 *  File:	    $RCSfile$
 *  Date modified:  $Date$
 *  Version:	    $Revision$
 *
 *
 *                                VR Juggler
 *                                    by
 *                              Allen Bierbaum
 *                             Christopher Just
 *                             Patrick Hartling
 *                            Carolina Cruz-Neira
 *                               Albert Baker
 *
 *                         Copyright  - 1997,1998,1999
 *                Iowa State University Research Foundation, Inc.
 *                            All Rights Reserved
 */


/*
 * --------------------------------------------------------------------------
 * Author:
 *   Patrick Hartling (based on vjMutexSGI by Allen Bierbaum).
 * --------------------------------------------------------------------------
 * NOTES:
 *    - This file (vjMutexPosix.h) must be included by vjMutex.h, not the
 *      other way around.
 *    - The following libraries must be linked in at compile time:
 *         HP-UX 10.20 --> -lcma
 *         IRIX 6.x    --> -lpthread
 * --------------------------------------------------------------------------
 */

#ifndef _VJ_MUTEX_POSIX_H_
#define _VJ_MUTEX_POSIX_H_

#include <vjConfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


//: Mutex wrapper for POSIX-compliant systems using pthreads mutex variables
//+ for the implementation.

//!PUBLIC_API:
class vjMutexPosix {
public:
    // -----------------------------------------------------------------------
    //: Constructor for vjMutexPosix class.
    //
    //! PRE: None.
    //! POST: The mutex variable is initialized for use.  It must be
    //+       initialized before any other member functions can do anything
    //+       with it.
    // -----------------------------------------------------------------------
    vjMutexPosix (void) {
        // ----- Allocate the mutex ----- //
        mMutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));

#ifdef _PTHREADS_DRAFT_4
        pthread_mutex_init(mMutex, pthread_mutexattr_default);
#else
        pthread_mutex_init(mMutex, NULL);
#endif
    }

    // -----------------------------------------------------------------------
    //: Destructor for vjMutexPosix class.
    //
    //! PRE: The mutex variable should be unlocked before being destroyed,
    //+      but if it is not, this routine will unlock it and then destroy
    //+      it.
    //! POST: The mutex variable is destroyed and unlocked if necessary.
    // -----------------------------------------------------------------------
    ~vjMutexPosix (void) {
        // Destroy the mutex.
        if ( pthread_mutex_destroy(mMutex) == -1 ) {
            pthread_mutex_unlock(mMutex);
            pthread_mutex_destroy(mMutex);
        }
    }

    // -----------------------------------------------------------------------
    //: Lock the mutex.
    //
    //! PRE: None.
    //! POST: A lock on the mutex variable is acquired by the caller.  If a
    //+       lock has already been acquired by another process/thread, the
    //+       caller blocks until the mutex has been freed.
    //
    //! RETURNS:  1 - Lock acquired
    //! RETURNS: -1 - Error
    // -----------------------------------------------------------------------
    inline int
    acquire (void) const {
        if ( pthread_mutex_lock(mMutex) == 0 ) {
            return 1;
        } else {
            return -1;
        }
    }

    // -----------------------------------------------------------------------
    //: Acquire a read mutex lock.
    //
    //! PRE: None.
    //! POST: A lock on the mutex variable is acquired by the caller.  If a
    //+       lock has already been acquired by another process/thread, the
    //+       caller blocks until the mutex has been freed.
    //
    //! RETURNS:  1 - Lock acquired
    //! RETURNS: -1 - Error
    //
    //! NOTE: No special read mutex has been defined for now.
    // -----------------------------------------------------------------------
    inline int
    acquireRead (void) const {
        return this->acquire();
    }

    // -----------------------------------------------------------------------
    //: Acquire a write mutex lock.
    //
    //! PRE: None.
    //! POST: A lock on the mutex variable is acquired by the caller.  If a
    //+       lock has already been acquired by another process/thread, the
    //+       caller blocks until the mutex has been freed.
    //
    //! RETURNS:  1 - Acquired
    //! RETURNS: -1 - Error
    //
    //! NOTE: No special write mutex has been defined for now.
    // -----------------------------------------------------------------------
    inline int
    acquireWrite (void) const {
        return this->acquire();
    }

    // -----------------------------------------------------------------------
    //: Try to acquire a lock on the mutex variable (does not block).
    //
    //! PRE: None.
    //! POST: A lock on the mutex variable is acquired by the caller.  If a
    //+       lock has already been acquired by another process/thread, the
    //+       caller returns does not wait for it to be unlocked.
    //
    //! RETURNS: 1 - Acquired
    //! RETURNS: 0 - Mutex is busy
    // -----------------------------------------------------------------------
    inline int
    tryAcquire (void) const {
        if ( pthread_mutex_trylock(mMutex) == 0 ) {
            return 1;
        } else {
            return 0;
        }
    }

    // -----------------------------------------------------------------------
    //: Try to acquire a read mutex lock (does not block).
    //
    //! PRE: None.
    //! POST: A lock on the mutex variable is acquired by the caller.  If a
    //+       lock has already been acquired by another process/thread, the
    //+       caller returns does not wait for it to be unlocked.
    //
    //! RETURNS: 1 - Acquired
    //! RETURNS: 0 - Mutex is busy
    // -----------------------------------------------------------------------
    inline int
    tryAcquireRead (void) const {
        return this->tryAcquire();
    }

    // -----------------------------------------------------------------------
    //: Try to acquire a write mutex (does not block)..
    //
    //! PRE: None.
    //! POST: A lock on the mutex variable is acquired by the caller.  If a
    //+       lock has already been acquired by another process/thread, the
    //+       caller returns does not wait for it to be unlocked.
    //
    //! RETURNS: 1 - Acquired
    //! RETURNS: 0 - Mutex is busy
    // -----------------------------------------------------------------------
    inline int
    tryAcquireWrite (void) const {
        return this->tryAcquire();
    }

    // -----------------------------------------------------------------------
    //: Release the mutex.
    //
    //! PRE: The mutex variable must be locked.
    //! POST: The mutex variable is unlocked.
    //
    //! RETURNS:  0 - Success
    //! RETURNS: -1 - Error
    // -----------------------------------------------------------------------
    inline int
    release (void) const {
        return pthread_mutex_unlock(mMutex);
    }

    // -----------------------------------------------------------------------
    //: Test the current lock status.
    //
    //! PRE: None.
    //! POST: The state of the mutex variable is returned.
    //
    //! RETURNS: 0 - Not locked
    //! RETURNS: 1 - Locked
    // -----------------------------------------------------------------------
    int test(void);

    // -----------------------------------------------------------------------
    //: Dump the mutex debug stuff and current state.
    //
    //! PRE: None.
    //! POST: All important data and debugging information related to the
    //+       mutex are dumped to the specified file descriptor (or to stderr
    //+       if none is given).
    //
    //! ARGS: dest - File descriptor to which the output will be written.
    //+              It defaults to stderr if no descriptor is specified.
    //! ARGS: message - Message printed out before the output is dumped.
    // -----------------------------------------------------------------------
    void
    dump (FILE* dest = stderr,
          const char* message = "\n------ Mutex Dump -----\n") const
    {
        fprintf(dest, "%s", message);
        fprintf(dest, "This is not currently implemented ...\n");
        // More needed ...
    }


// Allow the vjCondPosix class to access the private and protected members of
// this class.  Specifically, direct access is needed to the mutex variable.
friend class  vjCondPosix;

protected:
    pthread_mutex_t* mMutex;	//: Mutex variable for the class.

    // = Prevent assignment and initialization.
    void operator= (const vjMutexPosix &) {}
    vjMutexPosix (const vjMutexPosix &) {}
};


#endif	/* ifdef _VJ_MUTEX_POSIX_H_ */
