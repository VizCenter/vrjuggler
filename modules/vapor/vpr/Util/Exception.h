/****************** <VPR heading BEGIN do not edit this line> *****************
 *
 * VR Juggler Portable Runtime
 *
 * Original Authors:
 *   Allen Bierbaum, Patrick Hartling, Kevin Meinert, Carolina Cruz-Neira
 *
 ****************** <VPR heading END do not edit this line> ******************/

/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2006 by Iowa State University
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
 *************** <auto-copyright.pl END do not edit this line> ***************/

#ifndef _VPR_EXCEPTION_H_
#define _VPR_EXCEPTION_H_

#include <vpr/vprConfig.h>

#include <stdexcept>

//#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/stringize.hpp>

#define VPR_LOCATION std::string(__FILE__) + std::string(":") + \
           std::string(BOOST_PP_STRINGIZE(__LINE__))

// Example:
//  throw vpr::Exception("ObjectProxy not ready in Node::objProxy_checked", VPR_LOCATION);

// Exception areas
// - I/O loading/saving issues
// - Property access errors
// - Invalid data type errors

namespace vpr
{

/**
 * Base exception for all VPR exceptions.
 *
 * @since 1.1.5
 */
class VPR_CLASS_API Exception : public std::runtime_error
{
public:
   Exception(std::string desc, std::string location) throw();
   virtual ~Exception() throw();

   virtual const char* what() const throw();

   virtual std::string getExceptionName() const;

   std::string getDescription() const;
   void setDescription(std::string desc);

   std::string getLocation() const;
   std::string getStackTrace() const;

   /** Slightly longer description */
   virtual std::string getExtendedDescription() const;

   /** Description with everything we know */
   virtual std::string getFullDescription() const;

protected:
   std::string mDescription;
   std::string mLocation;
   std::string mStackTrace;

   mutable std::string m_full_desc;    /**< Temporary string to return as char* where needed */
};

}


#endif
