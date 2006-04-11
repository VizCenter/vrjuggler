/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2003 by Iowa State University
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

#ifndef _VRJ_KERNEL_EXCEPTIONS_H_
#define _VRJ_KERNEL_EXCEPTIONS_H_

#include <exception>
#include <string>


namespace vrj
{

/**
 * General exception type for managers held by vrj::Kernel.
 */
class ManagerException : public std::exception
{
public:
   ManagerException(const std::string& msg) : mMessage(msg)
   {
      ;
   }

   virtual ~ManagerException() throw()
   {
      ;
   }

   virtual const char* what() const throw()
   {
      return mMessage.c_str();
   }

protected:
   std::string mMessage;
};

/**
 * Exception type to be used by Draw Managers.
 */
class DrawMgrException : public ManagerException
{
public:
   DrawMgrException(const std::string& msg) : ManagerException(msg)
   {
      ;
   }

   virtual ~DrawMgrException() throw()
   {
      ;
   }
};

}

#endif /* _VRJ_KERNEL_EXCEPTIONS_H_ */
