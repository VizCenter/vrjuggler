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

#ifndef _VPR_SOCKET_DATAGRAM_IMPL_BSD_H_
#define _VPR_SOCKET_DATAGRAM_IMPL_BSD_H_

#include <string>

#include <vpr/md/POSIX/IO/Socket/SocketImplBSD.h>


namespace vpr {

/**
 * Implementation class for datagram sockets using the BSD sockets interface.
 *
 * @author Patrick Hartling
 */
class SocketDatagramImplBSD : public SocketImplBSD {
public:
    typedef SocketImplBSD Parent;

    // ========================================================================
    // vpr::SocketDatagram implementation.
    // ========================================================================

    /**
     * Default constructor.  This sets the socket type to
     * vpr::SocketTypes::DATAGRAM.
     */
    SocketDatagramImplBSD (void)
        : SocketImplBSD(SocketTypes::DATAGRAM)
    {
        /* Do nothing. */ ;
    }

    /**
     * Constructs a datagram socket using the given addresses as defaults for
     * communication channels.
     *
     * @post The member variables are initialized with the type in particular
     *       set to vpr::SocketTypes::DATAGRAM.
     *
     * @param local_addr  The local address for this socket.  This is used for
     *                    binding the socket.
     * @param remote_addr The remote address for this socket.  This is used to
     *                    specify a default destination for all packets.
     */
    SocketDatagramImplBSD (const InetAddr& local_addr,
                           const InetAddr& remote_addr)
        : SocketImplBSD(local_addr, remote_addr, SocketTypes::DATAGRAM)
    {
        /* Do nothing. */ ;
    }

    /**
     * Copy constructor.
     *
     * @post This socket is a copy of the given socket.
     */
    SocketDatagramImplBSD (const SocketDatagramImplBSD& sock)
        : SocketImplBSD(SocketTypes::DATAGRAM)
    {
        m_local_addr      = sock.m_local_addr;
        m_remote_addr     = sock.m_remote_addr;
        m_handle          = new FileHandleImplUNIX(sock.m_handle->getName());
        m_handle->m_fdesc = sock.m_handle->m_fdesc;
    }

    /**
     * Destructor.  This currently does nothing.
     */
    virtual ~SocketDatagramImplBSD (void) {
        /* Do nothing. */ ;
    }

    /**
     * Receives a message from the specified address.
     */
    virtual Status recvfrom(void* msg, const size_t length, const int flags,
                            InetAddr& from, ssize_t& bytes_read,
                            const vpr::Interval timeout = vpr::Interval::NoTimeout);

    /**
     * Sends a message to the specified address.
     */
    virtual Status sendto(const void* msg, const size_t length,
                          const int flags, const InetAddr& to,
                          ssize_t& bytes_sent,
                          const vpr::Interval timeout = vpr::Interval::NoTimeout);
};

}; // End of namespace


#endif	/* _VPR_SOCKET_DATAGRAM_IMPL_BSD_H_ */
