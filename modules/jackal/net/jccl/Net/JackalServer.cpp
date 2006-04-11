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

#include <jccl/jcclConfig.h>
#include <jccl/Net/JackalControl.h>
#include <jccl/Net/Connect.h>
#include <jccl/Config/ChunkDescDB.h>
#include <jccl/Config/ChunkDesc.h>
#include <jccl/Config/ConfigChunkDB.h>
#include <jccl/Config/ConfigChunk.h>
#include <jccl/Config/ParseUtil.h>
#include <jccl/Util/Debug.h>
#include <jccl/Net/JackalServer.h>

namespace jccl {

    vprSingletonImp(JackalServer);

    JackalServer::JackalServer():
        connections(),
        jackal_controls(),
        connections_mutex(),
        jackal_controls_mutex() {

        /* I want some hardcoded defaults, yes? */
        Port = 4450;
        listen_thread = NULL;
        listen_socket = NULL;
        configured_to_accept = false;
    }



    JackalServer::~JackalServer() {
        rejectConnections();
        connections_mutex.acquire();
        killConnections();
        connections_mutex.release();
    }



    bool JackalServer::isAccepting() {
        return (listen_thread != NULL);
    }



    void JackalServer::addJackalControl (JackalControl* jc) {
        vprASSERT (jc != NULL);

        jackal_controls_mutex.acquire();
        jackal_controls.push_back (jc);
        // eeks... scary deadlock warnings are going off in my head...
        connections_mutex.acquire();
        for (unsigned int i = 0, n = connections.size(); i < n; i++)
            jc->addConnect (connections[i]);
        connections_mutex.release();
        jackal_controls_mutex.release();

    }


    void JackalServer::removeJackalControl (JackalControl* jc) {
        vprASSERT (jc != NULL);

        std::vector<JackalControl*>::iterator it;
        jackal_controls_mutex.acquire();
        for (it = jackal_controls.begin(); it != jackal_controls.end(); it++) {
            if (*it == jc) {
                jackal_controls.erase(it);
                break;
            }
        }
        jackal_controls_mutex.release();
    }



    //: tells EM that a connection has died (ie by gui disconnecting)
    //  not for the case of removal by configRemove
    void JackalServer::connectHasDied (Connect* con) {
        std::string s = con->getName();

        jackal_controls_mutex.acquire();
        unsigned int i = 0, n = jackal_controls.size();
        for (; i < n; i++)
            jackal_controls[i]->removeConnect (con);
        jackal_controls_mutex.release();

        connections_mutex.acquire();
        removeConnection(con);
        connections_mutex.release();
    }



    std::vector<Connect*>& JackalServer::getConnections() {
        connections_mutex.acquire();
        return connections;
    }

    void JackalServer::releaseConnections() {
        connections_mutex.release();
    }



    //: ConfigChunkHandler stuff
    //! PRE: configCanHandle(chunk) == true
    //! RETURNS: success
    bool JackalServer::configAdd(ConfigChunkPtr chunk) {
        bool networkingchanged = false;
        int newport;

        std::string s = chunk->getDescToken();
        if ( s == std::string("EnvironmentManager") ||
             s == std::string("JackalServer") )
        {
            configured_to_accept = chunk->getProperty<bool>("AcceptConnections");
            newport = chunk->getProperty<int>("Port");

            if (newport == 0)
                newport = Port;
            if ((newport != Port) || (configured_to_accept != isAccepting()))
                networkingchanged = true;

            connections_mutex.acquire();
            if (networkingchanged) {
                Port = newport;
                if (isAccepting())
                    rejectConnections();  // close port w/ old config
                if (configured_to_accept)
                    acceptConnections();
                else
                    killConnections();
            }
            connections_mutex.release();

            return true;
        }
        else if ( s == std::string("FileConnect") ) {
            // I wanted to just look if the fileconnect had been added yet.
            // however I seem to have a chicken/egg problem.
            // so the kludge we'll do now is to not directly add a chunk that's
            // of type INTERACTIVE_CONNECT. sigh.
            // Unfortunately, this means that for other cases (such as attaching
            // to a named pipe) we're still broken
            if ( chunk->getProperty<int>("Mode") != INTERACTIVE_CONNECT ) {
                // it's new to us
                Connect* vn = new Connect (chunk);
                vprDEBUG (jcclDBG_SERVER, 1) << "EM adding connection: " << vn->getName().c_str() << '\n'
                                             << vprDEBUG_FLUSH;
                connections_mutex.acquire();
                connections.push_back (vn);
                vn->startProcess();
                connections_mutex.release();
            }
            return true;
        }
        return false;
    }



    //: Remove the chunk from the current configuration
    //! PRE: configCanHandle(chunk) == true
    //!RETURNS: success
    bool JackalServer::configRemove(ConfigChunkPtr chunk) {

        std::string s = chunk->getDescToken();
        if ( s == std::string("EnvironmentManager") ) {
            // this could be trouble if the chunk being removed isn't the chunk
            // we were configured with...
            rejectConnections();
            Port = 4450;
            configured_to_accept = false;
            return true;
        }
        else if ( s == std::string("FileConnect") ) {
            vprDEBUG (jcclDBG_SERVER,1) << "EM Removing connection: "
                                        << chunk->getName() << '\n'
                                        << vprDEBUG_FLUSH;
            connections_mutex.acquire();
            Connect* c = getConnection(chunk->getName());
            if (c) {
                removeConnection (c);
            }
            connections_mutex.release();
            vprDEBUG (jcclDBG_SERVER,4) << "EM completed connection removal\n" << vprDEBUG_FLUSH;
            return true;
        }

        return false;
    }



    //: Can the handler handle the given chunk?
    //! RETURNS: true - Can handle it
    //+          false - Can't handle it
    bool JackalServer::configCanHandle(ConfigChunkPtr chunk) {
        std::string s = chunk->getDescToken();
        return ( s == std::string("EnvironmentManager") ||
                 s == std::string("JackalServer") ||
                 s == std::string("FileConnect"));
    }



    //-------------------- PRIVATE MEMBER FUNCTIONS -------------------------

    // should only be called when we own connections_mutex
    void JackalServer::removeConnection (Connect* con) {
        vprASSERT (con != 0);

        std::vector<Connect*>::iterator i;
        for (i = connections.begin(); i != connections.end(); i++) {
            if (con == *i) {
                connections.erase (i);
                delete con;
                break;
            }
        }
    }



    // should only be called when we own connections_mutex
    Connect* JackalServer::getConnection (const std::string& s) {
        for (unsigned int i = 0; i < connections.size(); i++)
            if (s == connections[i]->getName())
                return connections[i];
        return NULL;
    }



    void JackalServer::controlLoop (void* nullParam) {
        // Child process used to listen for new network connections
        //struct sockaddr_in servaddr;
        Socket* servsock;
        Connect* connection;

        vprDEBUG(jcclDBG_SERVER,4) << "JackalServer started control loop.\n"
                                   << vprDEBUG_FLUSH;

        for (;;) {
            servsock = listen_socket->accept();
            char name[128];
            sprintf (name, "Network Connect %d", servsock->getID());
            vprDEBUG(jcclDBG_SERVER,vprDBG_CONFIG_LVL)
                << "JackalServer: Accepted connection: id: "
                << servsock->getID() << " on port: N/A\n" << vprDEBUG_FLUSH;
            connection = new Connect (servsock, (std::string)name);
            addConnection (connection);
        }
    }


    void JackalServer::addConnection (Connect* c) {
        connections_mutex.acquire();
        connections.push_back (c);
        connections_mutex.release();

        jackal_controls_mutex.acquire();
        unsigned int i = 0, n = jackal_controls.size();
        for (; i < n; i++)
            jackal_controls[i]->addConnect (c);
        jackal_controls_mutex.release();

        c->startProcess();
    }


    bool JackalServer::acceptConnections() {

        if (listen_thread != NULL)
            return true;

        listen_socket = new Socket ();
        if (!listen_socket->listen (Port)) {
            vprDEBUG(vprDBG_ERROR,vprDBG_CRITICAL_LVL) <<  clrOutNORM(clrRED,"ERROR:") << "Jackal Server couldn't open socket\n"
                                                       << vprDEBUG_FLUSH;
            return false;
        }
        else
            vprDEBUG(vprDBG_ALL,vprDBG_CRITICAL_LVL)
                << clrOutNORM(clrCYAN, "Jackal Server")
                << " listening on port " << clrOutNORM(clrMAGENTA, Port) << "\n"
                << vprDEBUG_FLUSH;

        /* now we ought to spin off a thread to do the listening */
        vpr::ThreadMemberFunctor<JackalServer>* memberFunctor =
            new vpr::ThreadMemberFunctor<JackalServer>(this,
                                                       &JackalServer::controlLoop,
                                                       NULL);
        listen_thread = new vpr::Thread(memberFunctor);

        return (listen_thread != NULL);
    }




    bool JackalServer::rejectConnections () {
        if (listen_thread) {
            listen_thread->kill();
            listen_thread = NULL;
            delete listen_socket;
            listen_socket = 0;
        }
        return 1;
    }



    // should only be called while we own connections_mutex
    void JackalServer::killConnections() {
        unsigned int i;

        //connections_mutex.acquire();
        for (i = 0; i < connections.size(); i++) {
            connections[i]->stopProcess();
            delete (connections[i]);
        }
        connections.erase (connections.begin(), connections.end());
        //connections_mutex.release();
    }

};

