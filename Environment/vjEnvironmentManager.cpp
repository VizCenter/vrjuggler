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


// implementation of Environment Manager
//
// author: Christopher Just


// needed for network cruft
#include <strings.h>    // For bzero()
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <Environment/vjEnvironmentManager.h>
#include <Kernel/vjKernel.h>
#include <Environment/vjConnect.h>
#include <Performance/vjPerfDataBuffer.h>
#include <Config/vjChunkDescDB.h>
#include <Config/vjConfigChunkDB.h>
#include <Config/vjParseUtil.h>
#include <Environment/vjTimedUpdate.h>

vjEnvironmentManager::vjEnvironmentManager():
                          connections(),
                          //updaters(),
                          perf_buffers(),
                          connections_mutex() {

    /* I want some hardcoded defaults, yes? */
    Port = 4450;
    listen_thread = NULL;
    listen_socket = -1;
    configured_to_accept = false;
    perf_refresh_time = 500;

    perf_target_name = "";
    perf_target = NULL;
    current_perf_config = NULL;
}



vjEnvironmentManager::~vjEnvironmentManager() {
    rejectConnections();
    connections_mutex.acquire();
    killConnections();
    connections_mutex.release();
}



bool vjEnvironmentManager::isAccepting() {
    return (listen_thread != NULL);
}



void vjEnvironmentManager::addPerfDataBuffer (vjPerfDataBuffer *b) {
    vjDEBUG (vjDBG_PERFORMANCE, 4) << "EM adding perf data buffer " << b->getName() << "\n"
		<< vjDEBUG_FLUSH;
    perf_buffers.push_back(b);
    activatePerfBuffers();
}




void vjEnvironmentManager::removePerfDataBuffer (vjPerfDataBuffer *b) {
    std::vector<vjPerfDataBuffer*>::iterator it;

    vjDEBUG (vjDBG_PERFORMANCE, 4) << "EM removing perf data buffer " << b->getName() 
				   << "\n" << vjDEBUG_FLUSH;

    b->deactivate();
    if (perf_target)
	perf_target->removeTimedUpdate (b);
    // this is one of those things I really hate:
    for (it = perf_buffers.begin(); it != perf_buffers.end(); it++) {
        if (*it == b) {
            perf_buffers.erase(it);
            break;
        }
    }

}



//: tells EM that a connection has died (ie by gui disconnecting)
//  not for the case of removal by configRemove
void vjEnvironmentManager::connectHasDied (vjConnect* con) {
    std::string s = con->getName();

    connections_mutex.acquire();
    removeConnect(con);
    connections_mutex.release();
    vjKernel::instance()->getChunkDB()->removeNamed(s);
    sendRefresh();
}



void vjEnvironmentManager::sendRefresh() {
    connections_mutex.acquire();
    for (int i = 0; i < connections.size(); i++) {
	connections[i]->sendRefresh();
    }
    connections_mutex.release();
}



//: ConfigChunkHandler stuff
//! PRE: configCanHandle(chunk) == true
//! RETURNS: success
bool vjEnvironmentManager::configAdd(vjConfigChunk* chunk) {
    bool networkingchanged = false;
    int newport;

    std::string s = chunk->getType();
    if (!vjstrcasecmp (s, "EnvironmentManager")) {
	configured_to_accept = chunk->getProperty ("AcceptConnections");
	newport = chunk->getProperty("Port");

	if (newport == 0)
	    newport = Port;
	if ((newport != Port) || (configured_to_accept != isAccepting()))
	    networkingchanged = true;
	perf_target_name = (std::string)chunk->getProperty ("PerformanceTarget");
	connections_mutex.acquire();
	
	vjConnect* new_perf_target = getConnect(perf_target_name);
	if (new_perf_target != perf_target)
	    setPerformanceTarget (NULL);

	if (networkingchanged) {
	    Port = newport;
	    if (isAccepting())
		rejectConnections();
	    if (configured_to_accept)
		acceptConnections();
	    else
		killConnections();
	}
	if (new_perf_target)
	    setPerformanceTarget(new_perf_target);
	connections_mutex.release();
	
	return true;
    }
    else if (!vjstrcasecmp (s, "PerfMeasure")) {
	current_perf_config = new vjConfigChunk (*chunk);
	activatePerfBuffers();
	return true;
    }
    else if (!vjstrcasecmp (s, "FileConnect")) {
	// I wanted to just look if the fileconnect had been added yet.
	// however I seem to have a chicken/egg problem.
	// so the kludge we'll do now is to not directly add a chunk that's
	// of type VJC_INTERACTIVE. sigh.
	// Unfortunately, this means that for other cases (such as attaching
	// to a named pipe) we're still broken
	if ((int)chunk->getProperty("Mode") != VJC_INTERACTIVE) {
	    // it's new to us
	    vjConnect* vn = new vjConnect (chunk);
	    vjDEBUG (vjDBG_ENV_MGR, 1) << "EM adding connection: " << vn->getName() << '\n'
				       << vjDEBUG_FLUSH;
	    connections_mutex.acquire();
	    connections.push_back (vn);
	    vn->startProcess();
	    if (!vjstrcasecmp (vn->getName(), perf_target_name))
		setPerformanceTarget (vn);
	    connections_mutex.release();
	}
	return true;
    }
    return false;
}



//: Remove the chunk from the current configuration
//! PRE: configCanHandle(chunk) == true
//!RETURNS: success
bool vjEnvironmentManager::configRemove(vjConfigChunk* chunk) {

    std::string s = chunk->getType();
    if (!vjstrcasecmp (s, "EnvironmentManager")) {
	// this could be trouble if the chunk being removed isn't the chunk
	// we were configured with...
	rejectConnections();
	Port = 4450;
	configured_to_accept = false;
	return true;
    }
    else if (!vjstrcasecmp (s, "PerfMeasure")) {
	if (current_perf_config) {
	    if (!vjstrcasecmp (current_perf_config->getProperty ("Name"),
			       chunk->getProperty ("Name"))) {
		delete (current_perf_config);
		current_perf_config = NULL;
		connections_mutex.acquire();
		deactivatePerfBuffers ();
		connections_mutex.release();
	    }
	}
	return true;
    }
    else if (!vjstrcasecmp (s, "FileConnect")) {
	vjDEBUG (vjDBG_ENV_MGR,1) << "EM Removing connection: " 
				  << chunk->getProperty ("Name") << '\n' << vjDEBUG_FLUSH;
	connections_mutex.acquire();
 	vjConnect* c = getConnect (chunk->getProperty ("Name"));
 	if (c) {
 	    removeConnect (c);
 	}
	connections_mutex.release();
	vjDEBUG (vjDBG_ENV_MGR,4) << "EM completed connection removal\n" << vjDEBUG_FLUSH;
	return true;
    }

    return false;
}



//: Can the handler handle the given chunk?
//! RETURNS: true - Can handle it
//+          false - Can't handle it
bool vjEnvironmentManager::configCanHandle(vjConfigChunk* chunk) {
    std::string s = chunk->getType();
    return (!vjstrcasecmp (s, "EnvironmentManager") ||
	    !vjstrcasecmp (s, "PerfMeasure") ||
	    !vjstrcasecmp (s, "FileConnect"));
}



//-------------------- PRIVATE MEMBER FUNCTIONS -------------------------

// should only be called when we own connections_mutex
void vjEnvironmentManager::removeConnect (vjConnect* con) {
    if (!con)
	return;
    if (con == perf_target)
	setPerformanceTarget (NULL);
    std::vector<vjConnect*>::iterator i;
    for (i = connections.begin(); i != connections.end(); i++)
	if (con == *i) {
	    connections.erase (i);
	    delete con;
	    break;
	}
}



// should only be called when we own connections_mutex
void vjEnvironmentManager::setPerformanceTarget (vjConnect* con) {
    if (con == perf_target)
	return;
    deactivatePerfBuffers();
    perf_target = con;
    activatePerfBuffers();
}



// should only be called when we own connections_mutex
vjConnect* vjEnvironmentManager::getConnect (const std::string& s) {
    for (int i = 0; i < connections.size(); i++)
	if (s == connections[i]->getName())
	    return connections[i];
    return NULL;
}



void vjEnvironmentManager::controlLoop (void* nullParam) {
    // Child process used to listen for new network connections
    struct sockaddr_in servaddr;
    int servsock;
    int len;
    vjConnect* connection;

    vjDEBUG(vjDBG_ENV_MGR,4) << "vjEnvironmentManager started control loop.\n"
	       << vjDEBUG_FLUSH;

    /* start listening for connections */
    if (listen (listen_socket, 0)) {
	vjDEBUG(vjDBG_ERROR,0) << "ERROR: vjEnvironmentManager socket listen "
		   << "failed\n" << vjDEBUG_FLUSH;
	return;
    }

    for (;;) {
	len = sizeof (struct sockaddr_in);
	servsock = accept (listen_socket,
			   (sockaddr*)&servaddr, &len);
	char name[128];
	sprintf (name, "Network Connect %d", servsock);
	connection = new vjConnect (servsock, (std::string)name);
	connections_mutex.acquire();
	connections.push_back( connection );
	connection->startProcess();
	connections_mutex.release();
    }
}



// should only be called while we have the connections mutex...
void vjEnvironmentManager::deactivatePerfBuffers () {
    std::vector<vjPerfDataBuffer*>::iterator i;
    for (i = perf_buffers.begin(); i != perf_buffers.end(); i++) {
	(*i)->deactivate();
	if (perf_target)
	    perf_target->removeTimedUpdate (*i);
    }
}


// should only be called while we own connections_mutex
void vjEnvironmentManager::activatePerfBuffers () {
    // activates all perf buffers configured to do so
    // this is still a bit on the big and bulky side.

    if (perf_buffers.empty())
	return;

    if (perf_target == NULL || current_perf_config == NULL) {
	deactivatePerfBuffers();
	return;
    }

    std::vector<vjVarValue*> v = current_perf_config->getAllProperties ("TimingTests");
    std::vector<vjPerfDataBuffer*>::const_iterator b;
    std::vector<vjVarValue*>::const_iterator val;
    bool found;
    vjConfigChunk* ch;

    for (b = perf_buffers.begin(); b != perf_buffers.end(); b++) {
	found = false;
	for (val = v.begin(); val != v.end(); val++) {
	    ch = *(*val); // this line demonstrates a subtle danger
	    if ((bool)ch->getProperty ("Enabled")) {
		if (!vjstrncasecmp(ch->getProperty("Prefix"), (*b)->getName()))
		    found = true;
	    }
	}
	if (found) {
	    (*b)->activate();
	    perf_target->addTimedUpdate ((*b), perf_refresh_time);
	}
	else if ((*b)->isActive()) {
	    (*b)->deactivate();
	    perf_target->removeTimedUpdate (*b);
	}
    }
    for (val = v.begin(); val != v.end(); val++) {
	delete (*val);
    }

}



bool vjEnvironmentManager::acceptConnections() {
    struct sockaddr_in sockaddress;

    if (listen_thread != NULL)
	return true;

    /* here, we open a socket & get ready to read connections */
    listen_socket = socket (AF_INET, SOCK_STREAM, 0);
    bzero(&sockaddress, sizeof (struct sockaddr_in));
    sockaddress.sin_family = PF_INET;
    sockaddress.sin_port = htons(Port);

    if (bind ( listen_socket, (sockaddr*)&sockaddress,
	       sizeof (struct sockaddr_in))) {
	vjDEBUG(vjDBG_ERROR,0) << "ERROR: Environment Manager couldn't open socket\n"
		   << vjDEBUG_FLUSH;
	return false;
    }
    else
	vjDEBUG(vjDBG_ALL,0) << "Environment Manager accepting connections on port "
		   << Port << '\n' << vjDEBUG_FLUSH;

    /* now we ought to spin off a thread to do the listening */
    vjThreadMemberFunctor<vjEnvironmentManager>* memberFunctor =
	new vjThreadMemberFunctor<vjEnvironmentManager>(
				this,
				&vjEnvironmentManager::controlLoop,
	       			NULL);
    listen_thread = new vjThread (memberFunctor, 0);


    return (listen_thread != NULL);
}




bool vjEnvironmentManager::rejectConnections () {
    if (listen_thread) {
	listen_thread->kill();
	listen_thread = NULL;
	close(listen_socket);
    }
    return 1;
}



// should only be called while we own connections_mutex
void vjEnvironmentManager::killConnections() {
    int i;

    //connections_mutex.acquire();
    for (i = 0; i < connections.size(); i++) {
	connections[i]->stopProcess();
	delete (connections[i]);
    }
    connections.erase (connections.begin(), connections.end());
    //connections_mutex.release();
}



