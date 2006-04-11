/****************** <VPR heading BEGIN do not edit this line> *****************
 *
 * VR Juggler Portable Runtime
 *
 * Original Authors:
 *   Allen Bierbaum, Patrick Hartling, Kevin Meinert, Carolina Cruz-Neira
 *
 * -----------------------------------------------------------------
 * File:          $RCSfile$
 * Date modified: $Date$
 * Version:       $Revision$
 * -----------------------------------------------------------------
 *
 ****************** <VPR heading END do not edit this line> ******************/

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
 *************** <auto-copyright.pl END do not edit this line> ***************/

#ifndef _VPR_SERIALIZABLE_DEVICE_OBJ_H
#define _VPR_SERIALIZABLE_DEVICE_OBJ_H

#include <vpr/IO/SerializableObject.h>

namespace gadget
{
                              
class SerializableDevice //: public vpr::SerializableObject
{
public:
   SerializableDevice()
   {
     
   }
   virtual vpr::ReturnStatus writeObject(vpr::ObjectWriter* writer) = 0;
   virtual vpr::ReturnStatus readObject(vpr::ObjectReader* reader, vpr::Uint64* delta) = 0;
   /*
   * Template method for reading object.
   * POST: All object data is read from the reader
   */
protected:
   
   /** @link dependency 
    * @stereotype use*/
   /*#  ObjectReader lnkObjectReader; */
};

} // namespace vpr

#endif
