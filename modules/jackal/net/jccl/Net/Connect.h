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
 * -----------------------------------------------------------------
 * File:          $RCSfile$
 * Date modified: $Date$
 * Version:       $Revision$
 * -----------------------------------------------------------------
 *
 *************** <auto-copyright.pl END do not edit this line> ***************/
#ifndef _JCCL_CONNECT_H_
#define _JCCL_CONNECT_H_

#include <jccl/jcclConfig.h>
#include <jccl/Config/ConfigChunkPtr.h>
#include <jccl/Config/ConfigChunk.h>
#include <jccl/Net/Socket.h>
#include <jccl/Net/NetCommunicator.h>
#include <jccl/Net/Command.h>
#include <vpr/Thread/Thread.h>
#include <vpr/Util/Interval.h>

#include <queue>

namespace jccl {


enum ConnectMode { INTERACTIVE_CONNECT, INPUT_CONNECT, OUTPUT_CONNECT };

//--------------------------------------------------
//: vjConnect reads/writes to a file, pipe, or socket.
//
//
//---------------------------------------
class JCCL_CLASS_API Connect {
 public:

    Connect (Socket* s, const std::string& _name="unnamed", 
	       ConnectMode _mode = INTERACTIVE_CONNECT);



    Connect (ConfigChunkPtr c);


    //: destructor
    //! PRE:  True
    //! POST: all dynamically-allocated memory associated
    //+       with self is freed.
    //+       If ControlPID is non-NULL, the process it refers
    //+       to is stopped.
    ~Connect();



    //: returns the name of this connection
    //! NOTE: The name is the same as the name of the ConfigChunk that
    //+       represents it.
    std::string getName () {
	//cout << "name of this thing is " << name << endl;
	return name;
    }


    ConnectMode getConnectMode () const {
        return mode;
    }


    ConfigChunkPtr getConfiguration () {
        return connect_chunk;
    }


    void addCommunicator (NetCommunicator* c) {
        communicators.push_back (c);
        c->initConnection (this);
    }


    //: Starts the file connection process.
    //! PRE:  None
    //! POST: A thread has been created to read from the file/pipe
    //+       connection (or an error has occurred while
    //+       attempting to do so).
    //+       controlPID is set to the PID of the generated process.
    //! RETURNS: True - successfully created the thread.
    //! RETURNS: False - unable to create thread.
    bool startProcess();



    //: Stops file connection process.
    //! PRE:  None
    //! POST: If self had a thread associated with it, that
    //+       thread is terminated.
    //! RETURNS: always True
    bool stopProcess();


    void sendDisconnect();

    void addCommand (Command* cmd);

    //: Attaches a PeriodicCommand object to this connection
    //! ARGS: cmd - a non-NULL PeriodicCommand 
    void addPeriodicCommand (PeriodicCommand* cmd);

    //: Detaches a PeriodicCommand object from this connection
    void removePeriodicCommand (PeriodicCommand* cmd);


private:


    std::ostream*     outstream;
    std::istream*      instream;
    bool                    shutdown;        // set to stop procs
    std::string             name;
    std::string             filename;
    vpr::Thread*               read_connect_thread;
    vpr::Thread*               write_connect_thread;
    Socket*               sock;
    ConnectMode           mode;
    bool                    read_die;    // if true, thread suicide
    bool                    write_die;   // if true, thread suicide
    bool                    write_alive; // true when thread running
    ConfigChunkPtr          connect_chunk;

    //: used for storing Command* in a priority queue
    struct CommandPtrCmp {
	bool operator() (const PeriodicCommand* a, const PeriodicCommand* b) {
	    return (a->next_fire_time > b->next_fire_time);
	}
    };


    std::priority_queue<PeriodicCommand*, std::vector<PeriodicCommand*>, CommandPtrCmp>
                               periodic_commands; // used as heap
    std::queue<Command*>     commands;

    //: controls access to commands & periodic_commands queues.
    //  could we dispense with this???
    vpr::Mutex                    commands_mutex;

    //: used to see if it's time to spring a timed_command
    vpr::Interval             current_time;

    //: body of network process.
    void readControlLoop (void* nullParam);
    void writeControlLoop (void* nullParam);

    //: utility for controlLoop()
    bool readCommand (std::istream& fin);

    std::vector<NetCommunicator*> communicators;

    // These are needed to appease Visual C++ in its creation of DLLs.
    Connect(const Connect&) {;}
    void operator=(const Connect&) {;}

}; // end Connect

};

#endif
