/*
 * --------------------------------------------------------------------------
 * vjSemaphorePosix.h
 * $Revision$
 * $Date$
 *
 * Author:
 *   Patrick Hartling (based on vjSemaphoreSGI by Allen Bierbaum).
 * --------------------------------------------------------------------------
 * NOTES:
 *    - This file (vjSemaphorePosix.h) must be included by vjSemaphore.h, not
 *      the other way around.
 * --------------------------------------------------------------------------
 */

#ifndef _SEMAPHORE_POSIX_H_
#define _SEMAPHORE_POSIX_H_

#include <config.h>
#include <unistd.h>
#include <limits.h>
#include <semaphore.h>
#include <errno.h>


//: Semaphore wrapper for POSIX.4-compliant systems.

class vjSemaphorePosix {
public:
    // -----------------------------------------------------------------------
    //: Custructor for vjSemaphorePosix class.
    //
    //! PRE: None.
    //! POST: The semaphore variable for the class is initilized as an
    //+       unnamed semaphore.
    //
    //! ARGS: initialValue - The initial number of resources controlled by
    //+                      the semaphore.  If not specified, the default
    //+                      value is 1.
    // -----------------------------------------------------------------------
    vjSemaphorePosix (int initialValue = 1) {
        // ----- Allocate the unnamed semaphore ----- //
        if ( sem_init(sema, 0, initialValue) != 0 ) {
            perror("sem_init() error");
        }
    }

    // -----------------------------------------------------------------------
    //: Destructor for vjSemaphorePosix class.
    //
    //! PRE: None.
    //! POST: The resources used by the semaphore variable are freed.
    // -----------------------------------------------------------------------
    ~vjSemaphorePosix (void) {
        // ---- Delete the semaphore --- //
        if ( sem_destroy(sema) != 0 ) {
            perror("sem_destroy() error");
        }
    }

    // -----------------------------------------------------------------------
    //: Lock the semaphore.
    //
    //! PRE: None.
    //! POST: The calling thread either acquires the semaphore until
    //+       release() is called, or the caller is put at the tail of a wait
    //+       and is suspended until such time as it can be freed and allowed
    //+       to acquire the semaphore itself.
    //
    //! RETURNS:  1 - Lock acquired
    //! RETURNS: -1 - Error
    // -----------------------------------------------------------------------
    inline int
    acquire (void) const {
        if ( sem_wait(sema) == 0 ) {
            return 1;
        } else {
            perror("sem_wait() error");
            return -1;
        }
    }

    // -----------------------------------------------------------------------
    //: Acquire and lock a read semaphore.
    //
    //! PRE: None.
    //! POST: The calling thread either acquires the semaphore until
    //+       release() is called, or the caller is put at the tail of a wait
    //+       and is suspended until such time as it can be freed and allowed
    //+       to acquire the semaphore itself.
    //
    //! RETURNS:  1 - Lock acquired
    //! RETURNS: -1 - Error
    //
    //! NOTE: There is no special read semaphore for now.
    // -----------------------------------------------------------------------
    inline int
    acquireRead (void) const {
        return this->acquire();
    }

    // -----------------------------------------------------------------------
    //: Aquire and lock a write semaphore.
    //
    //! PRE: None.
    //! POST: The calling thread either acquires the semaphore until
    //+       release() is called, or the caller is put at the tail of a wait
    //+       and is suspended until such time as it can be freed and allowed
    //+       to acquire the semaphore itself.
    //
    //! RETURNS:  1 - Lock acquired
    //! RETURNS: -1 - Error
    //
    //! NOTE: There is no special write semaphore for now.
    // -----------------------------------------------------------------------
    inline int
    acquireWrite (void) const {
        return this->acquire();
    }

    // -----------------------------------------------------------------------
    //: Try to acquire the semaphore immediately (does not block).
    //
    //! PRE: None.
    //! POST: If the semaphore could be acquired by the caller, the caller
    //+       gets control of the semaphore.  If the semaphore was already
    //+       locked, the routine returns immediately without suspending the
    //+       calling thread.
    //
    //! RETURNS: 1 - Acquired
    //! RETURNS: 0 - Not acquired
    // -----------------------------------------------------------------------
    inline int
    tryAcquire (void) const {
        return sem_trywait(sema);
    }

    // -----------------------------------------------------------------------
    //: Try to acquire a read semaphore (does not block).
    //
    //! PRE: None.
    //! POST: If the semaphore could be acquired by the caller, the caller
    //+       gets control of the semaphore.  If the semaphore was already
    //+       locked, the routine returns immediately without suspending the
    //+       calling thread.
    //
    //! RETURNS: 1 - Acquired
    //! RETURNS: 0 - Not acquired
    // -----------------------------------------------------------------------
    inline int
    tryAcquireRead (void) const {
        return this->tryAcquire();
    }

    // -----------------------------------------------------------------------
    //: Try to acquire a write semaphore (does not block).
    //
    //! PRE: None.
    //! POST: If the semaphore could be acquired by the caller, the caller
    //+       gets control of the semaphore.  If the semaphore was already
    //+       locked, the routine returns immediately without suspending the
    //+       calling thread.
    //
    //! RETURNS: 1 - Acquired
    //! RETURNS: 0 - Not acquired
    // -----------------------------------------------------------------------
    inline int
    tryAcquireWrite (void) const {
        return this->tryAcquire();
    }

    // -----------------------------------------------------------------------
    //: Release the semaphore.
    //
    //! PRE: The semaphore should have been locked before being released.
    //! POST: The semaphore is released and the thread at the haed of the
    //+       wait queue is allowed to execute again.
    //
    //! RETURNS:  0 - Success
    //! RETURNS: -1 - Error
    // -----------------------------------------------------------------------
    inline int
    release (void) const {
        return sem_post(sema);
    }

    // -----------------------------------------------------------------------
    //: Reset the semaphore.
    //
    //! PRE: None.
    //! POST: The semaphore's count is set to the specified value.
    //
    //! ARGS: val - The value to which the semaphore is reset.
    //
    //! RETURNS:  0 - Success
    //! RETURNS: -1 - Error
    //
    //! NOTE: If processes are waiting on the semaphore, the results are
    //+       undefined.
    // -----------------------------------------------------------------------
    inline int
    reset (int val) {
        // First destroy the current semaphore.
        sem_destroy(sema);

        // Now recreate it with the new value in val.
        return sem_init(sema, 0, val);
    }

    // -----------------------------------------------------------------------
    //: Dump the semaphore debug stuff and current state.
    //
    //! PRE: None.
    //! POST: All important data and debugging information related to the
    //+       semaphore is dumped to the specified file descriptor (or to
    //+       stderr if none is given).
    //
    //! ARGS: dest - File descriptor to which the output will be written.
    //+              It defaults to stderr if no descriptor is specified.
    //! ARGS: message - Message printed out before the output is dumped.
    // -----------------------------------------------------------------------
    void
    dump (FILE* dest = stderr,
          const char* message = "\n------ Semaphore Dump -----\n") const
    {
        int value;

        sem_getvalue(sema, &value);

        fprintf(dest, "%s", message);
        fprintf(dest, "Current semaphore value: %d", value);
    }

protected:
    sem_t* sema;	//: Semaphore variable for the class.

    // Prevent assignment and initialization.
    void operator= (const vjSemaphorePosix &) {}
    vjSemaphorePosix (const vjSemaphorePosix &) {}
};

#endif	/* _SEMAPHORE_POSIX_H_ */
