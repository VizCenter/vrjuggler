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

/*
 * --------------------------------------------------------------------------
 * NOTES:
 *    - This file (vprThreadKeyWin32.h) MUST be included by vprThread.h, not
 *      the other way around.
 * --------------------------------------------------------------------------
 */

#ifndef _VPR_THREAD_KEY_WIN_32_H_
#define _VPR_THREAD_KEY_WIN_32_H_

#include <vpr/vprConfig.h>

#include <stdio.h>

#include <vpr/Thread/ThreadFunctor.h>


namespace vpr {

// Key destructor function type.
typedef thread_func_t KeyDestructor;

class ThreadKeyWin32 {
public:
    // -----------------------------------------------------------------------
    //: Constructor.  Allocate a <keyp> that is used to identify data that is
    //+ specific to each thread in the process, is global to all threads in
    //+ the process and is destroyed using the spcefied destructor function
    //+ that takes a single argument.
    //
    //! PRE: None.
    //! POST: A key is created and is associated with the specified
    //+       destructor function and argument.
    //
    //! ARGS: dest_func - The destructor function for the key.  This uses
    //+                   the functor data structure.
    //! ARGS: arg - Argument to be passed to destructor (optional).
    // -----------------------------------------------------------------------
    ThreadKeyWin32 (thread_func_t destructor, void* arg = 0)
        : mKeyID(0xffffffff), mDestroyData(NULL)
    {
        mDestroyData = new ThreadNonMemberFunctor(destructor, arg);
        keycreate();
    }

    // -----------------------------------------------------------------------
    //: Constructor.  Allocates a <keyp> that is used to identify data that
    //+ is specific to each thread in the process, is global to all threads
    //+ in the process and is destroyed by the specified destructor function.
    //
    //! PRE: None.
    //! POST: A key is created and is associated with the specified
    //+       destructor function.
    //
    //! ARGS: desctructor - Procedure to be called to destroy a data value
    //+                     associated with the key when the thread
    //+                     terminates.
    // -----------------------------------------------------------------------
    ThreadKeyWin32 (BaseThreadFunctor* destructor)
        : mKeyID(0xffffffff), mDestroyData(NULL)
    {
        mDestroyData = destructor;
        keycreate();
    }

    // -----------------------------------------------------------------------
    //: Destructor.
    // -----------------------------------------------------------------------
    ~ThreadKeyWin32 (void) {
        keyfree();
    }

    // -----------------------------------------------------------------------
    //: Allocate a <keyp> that is used to identify data that is specific to
    //+ each thread in the process, is global to all threads in the process
    //+ and is destroyed using the spcefied destructor function that takes a
    //+ single argument.
    //
    //! PRE: None.
    //! POST: A key is created and is associated with the specified
    //+       destructor function and argument.
    //
    //! ARGS: dest_func - The destructor function for the key.  This uses
    //+                   the functor data structure.
    //! ARGS: arg - Argument to be passed to destructor (optional).
    //
    //! RETURNS:  0 - Successful completion
    //! RETURNS: -1 - Error
    //
    //! NOTE: Use this routine to construct the key destructor function if
    //+       it requires arguments.  Otherwise, use the two-argument
    //+       version of keycreate().
    // -----------------------------------------------------------------------
    inline int
    keycreate (thread_func_t destructor, void* arg = 0) {
        if ( mDestroyData != NULL ) {
            delete mDestroyData;
        }

        mDestroyData = new ThreadNonMemberFunctor(destructor, arg);
        return keycreate();
    }

    // -----------------------------------------------------------------------
    //: Allocates a <keyp> that is used to identify data that is specific to
    //+ each thread in the process, is global to all threads in the process
    //+ and is destroyed by the specified destructor function.
    //
    //! PRE: None.
    //! POST: A key is created and is associated with the specified
    //+       destructor function.
    //
    //! ARGS: desctructor - Procedure to be called to destroy a data value
    //+                     associated with the key when the thread
    //+                     terminates.
    //
    //! RETURNS:  0 - Successful completion
    //! RETURNS: -1 - Error
    // -----------------------------------------------------------------------
    inline int
    keycreate (BaseThreadFunctor* destructor) {
        mDestroyData = destructor;
        return keycreate();
    }

    // -----------------------------------------------------------------------
    //: Free up the key so that other threads may reuse it.
    //
    //! PRE: The specified key must have been properly created using the
    //+      keycreate() member function.
    //! POST: The specified key is destroyed using the destructor function
    //+       previously associated with it, and its resources are freed.
    //
    //! RETURNS:  0 - Successful completion
    //! RETURNS: -1 - Error
    // -----------------------------------------------------------------------
    inline int
    keyfree (void) {
        int retval;

        if ( mDestroyData != NULL ) {
            (*mDestroyData)();
        }

        retval = 0;

        if ( mKeyID != 0xffffffff ) {
           if ( ! TlsFree(mKeyID) ) {
               perror("Could not free thread local storage");
               retval = -1;
           }
        }

        return retval;
    }

    // -----------------------------------------------------------------------
    //: Bind value to the thread-specific data key, <key>, for the calling
    //+ thread.
    //
    //! PRE: The specified key must have been properly created using the
    //+      keycreate() member function.
    //! POST: The specified value is associated with the key for the calling
    //+       thread.
    //
    //! ARGS: value - Address containing data to be associated with the
    //+               specified key for the current thread.
    //
    //! RETURNS:  0 - Successful completion
    //! RETURNS: -1 - Error
    // -----------------------------------------------------------------------
    inline int
    setspecific (void* value) {
        int retval;

        retval = 0;
        if ( ! TlsSetValue(mKeyID, value) ) {
            perror("Could not set value for thread local storage");
            retval = -1;
        }

        return retval;
    }

    // -----------------------------------------------------------------------
    //: Stores the current value bound to <key> for the calling thread into
    //+ the location pointed to by <valuep>.
    //
    //! PRE: The specified key must have been properly created using the
    //+      keycreate() member function.
    //! POST: The value associated with the key is obtained and stored in the
    //+       pointer valuep so that the caller may work with it.
    //
    //! ARGS: valuep - Address of the current data value associated with the
    //+                key.
    //
    //! RETURNS:  0 - Successful completion
    //! RETURNS: -1 - Error
    // -----------------------------------------------------------------------
    inline int
    getspecific (void** valuep) {
        *valuep = TlsGetValue(mKeyID);

        return 0;
    }

private:
    // -----------------------------------------------------------------------
    //: Allocate a <keyp> that is used to identify data that is specific to
    //+ each thread in the process, is global to all threads in the process.
    // -----------------------------------------------------------------------
    inline int
    keycreate (void) {
        int retval;

        retval = 0;
        mKeyID = TlsAlloc();

        if ( mKeyID == 0xffffffff ) {
            perror("Could not create thread local storage id");
            retval = -1;
        }

        return retval;
    }

    DWORD mKeyID;                    //: Thread local storage ID
    BaseThreadFunctor* mDestroyData; //: Destructor for TLS data
};

}; // End of vpr namespace


#endif	/* _VPR_THREAD_KEY_WIN_32_H_ */
