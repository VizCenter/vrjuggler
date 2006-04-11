/*
 * --------------------------------------------------------------------------
 * vjThreadPosix.h
 * $Revision$
 * $Date$
 * --------------------------------------------------------------------------
 * NOTES:
 *    - This file (vjThreadPosix.h) MUST be included by vjThread.h, not the
 *      other way around.
 *    - The following libraries must be linked in at compile time:
 *         HP-UX 10.20 --> -lcma
 *         IRIX 6.x    --> -lpthread
 * --------------------------------------------------------------------------
 */

#ifndef _THREAD_POSIX_H_
#define _THREAD_POSIX_H_


#include <config.h>
#include <iostream.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>

#include <Threads/vjThreadFunctor.h>
#include <Threads/vjBaseThread.h>


//: Threads implementation using POSIX threads (both Draft 4 and the "final"
//+ draft of the standard are supported).

class vjThreadPosix : public vjBaseThread
{
public:
    // -----------------------------------------------------------------------
    //: Constructor.
    // -----------------------------------------------------------------------
    vjThreadPosix (void)
    {;}

    // -----------------------------------------------------------------------
    //: Create a new thread that will execute functorPtr.
    //
    //! PRE: None.
    //! POST: A thread (with any specified attributes) is created that begins
    //+       executing func().  Depending on the scheduler, it may being
    //+       execution immediately, or it may block for a short time before
    //+       beginning execution.
    //
    //! ARGS: functorPtr - Function to be executed by the thread.
    //! ARGS: flags - Flags for the thread--not currently used in this
    //+               implementation (optional).
    //! ARGS: priority - Priority of created thread (optional).
    //! ARGS: stack_addr - Alternate address for thread's stack (optional).
    //! ARGS: stack_size - Size for thread's stack (optional).
    //
    //! RETURNS:  0 - Successful thread creation
    //! RETURNS: -1 - Error
    //
    //! NOTE: The pthreads implementation on HP-UX 10.20 does not allow the
    //+       stack address to be changed.
    // -----------------------------------------------------------------------
    int spawn ( vjBaseThreadFunctor* functorPtr, long flags = 0,
                u_int priority = 0, void* stack_addr = NULL,
                size_t stack_size = 0);

    // -----------------------------------------------------------------------
    //: Make the calling thread wait for the termination of this thread.
    //
    //! PRE: None.
    //! POST: The caller blocks until this thread finishes its execution
    //+       (i.e., calls the exit() method).  This routine may return
    //+       immediately if this thread has already exited.
    //
    //! ARGS: status - Status value of the terminating thread when that
    //+                thread calls the exit routine (optional).
    //
    //! RETURNS:  0 - Successful completion
    //! RETURNS: -1 - Error
    // -----------------------------------------------------------------------
    virtual int
    join (void** status = 0) {
        return pthread_join(threadID, status);
    }


    // -----------------------------------------------------------------------
    //: Resume the execution of a thread that was previously suspended using
    //+ suspend().
    //
    //! PRE: This thread was previously suspended using the suspend() member
    //+      function.
    //! POST: This thread is sent the SIGCONT signal and is allowed to begin
    //+       executing again.
    //
    //! RETURNS:  0 - Successful completion
    //! RETURNS: -1 - Error
    //
    //! NOTE: This is not currently supported on HP-UX 10.20.
    // -----------------------------------------------------------------------
    virtual int resume (void)
    { return kill(SIGCONT); }

    // -----------------------------------------------------------------------
    //: Suspend the execution of this thread.
    //
    //! PRE: None.
    //! POST: This thread is sent the SIGSTOP signal and is thus suspended
    //+       from execution until the member function resume() is called.
    //
    //! RETURNS:  0 - Successful completion
    //! RETURNS: -1 - Error
    //
    //! NOTE: This is not currently supported on HP-UX 10.20.
    // -----------------------------------------------------------------------
    virtual int suspend (void)
    { return kill(SIGSTOP); }

    // -----------------------------------------------------------------------
    //: Get this thread's priority.
    //
    //! PRE: None.
    //! POST: The priority of this thread is returned in the integer pointer
    //+       variable.
    //
    //! ARGS: prio - Pointer to an int variable that will have the thread's
    //+              priority stored in it.
    //
    //! RETURNS:  0 - Successful completion
    //! RETURNS: -1 - Error
    //
    //! NOTE: This is only supported on systems that support thread priority
    //+       scheduling in their pthreads implementation.
    // -----------------------------------------------------------------------
    virtual int getPrio (int* prio);

    // -----------------------------------------------------------------------
    //: Set this thread's priority.
    //
    //! PRE: None.
    //! POST: This thread has its priority set to the specified value.
    //
    //! ARGS: prio - The new priority for this thread.
    //
    //! RETURNS:  0 - Successful completion
    //! RETURNS: -1 - Error
    //
    //! NOTE: This is only supported on systems that support thread priority
    //+       scheduling in their pthreads implementation.
    // -----------------------------------------------------------------------
    virtual int setPrio (int prio);

    // -----------------------------------------------------------------------
    //: Yield execution of the calling thread to allow a different blocked
    //+ thread to execute.
    //
    //! PRE: None.
    //! POST: The caller yields its execution control to another thread or
    //+       process.
    // -----------------------------------------------------------------------
    virtual void yield (void)
    { sched_yield();}


    // -----------------------------------------------------------------------
    //: Send the specified signal to this thread (not necessarily SIGKILL).
    //
    //! PRE: None.
    //! POST: This thread receives the specified signal.
    //
    //! ARGS: signum - The signal to send to the specified thread.
    //
    //! RETURNS:  0 - Successful completion
    //! RETURNS: -1 - Error
    //
    //! NOTE: This is not currently supported on HP-UX 10.20.
    // -----------------------------------------------------------------------
    virtual int
    kill (int signum) {
#ifdef _PTHREADS_DRAFT_4
        cerr << "vjThreadPosix::kill(): Signals cannot be sent to threads "
             << "with this POSIX threads implementation.\n";

        return -1;
#else
        return pthread_kill(threadID, signum);
#endif
    }

    // -----------------------------------------------------------------------
    //: Kill (cancel) this thread.
    //
    //! PRE: None.
    //! POST: This thread is cancelled.  Depending on the cancellation
    //+       attributes of the specified thread, it may terminate
    //+       immediately, it may wait until a pre-defined cancel point to
    //+       stop or it may ignore the cancel altogether.  Thus, immediate
    //+       cancellation is not guaranteed.
    //
    //! NOTE: For the sake of clarity, it is probably better to use the
    //+       cancel() routine instead of kill() because a two-argument
    //+       version of kill() is also used for sending signals to threads.
    //+       This kill() and cancel() do exactly the same thing.
    // -----------------------------------------------------------------------
    virtual void
    kill (void) {
        pthread_cancel(threadID);
    }


    // -----------------------------------------------------------------------
    //: Overload the << operator to allow printing of the process and thread
    //+ ID neatly.
    // -----------------------------------------------------------------------
    ostream& outStream (ostream& out) {
      out << "pThrd:[" << threadId << "]";
      return out;
    }

// Private member variables.
private:
    pthread_t	threadID;	//: pthread_t data structure for this thread
};


#endif	/* _THREAD_POSIX_H_ */
