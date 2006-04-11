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

#include <vpr/vprConfig.h>

#include <uuid/sysdep.h>
#include <uuid/uuid.h>

#include <stdio.h>
#include <string.h>

#include <vpr/Util/GUID.h>


namespace vpr
{

 vpr::GUID::StdGUID null_guid_struct = { 0x00000000, 0x0000, 0x0000, 0x00, 0x00,
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
      };

 const vpr::GUID GUID::NullGUID( null_guid_struct );

 vpr::GUID::GenerateTag GUID::generateTag;
//


std::string GUID::toString () const
{
   std::string guid_str;
   char guid_c_str[37];

#ifdef HAVE_SNPRINTF
   snprintf(guid_c_str, 37,
#else
   sprintf(guid_c_str,
#endif
            "%08X-%04hX-%04hX-%02X%02X-%02X%02X%02X%02X%02X%02X",
            mGuid.moz.m0, mGuid.moz.m1, mGuid.moz.m2, mGuid.moz.m3[0],
            (vpr::Uint32) mGuid.moz.m3[1], (vpr::Uint32) mGuid.moz.m3[2],
            (vpr::Uint32) mGuid.moz.m3[3], (vpr::Uint32) mGuid.moz.m3[4],
            (vpr::Uint32) mGuid.moz.m3[5], (vpr::Uint32) mGuid.moz.m3[6],
            (vpr::Uint32) mGuid.moz.m3[7]);
   guid_str = guid_c_str;

   return guid_str;
}

bool GUID::operator== (const GUID& guid) const
{
   /*
   bool ret_val0 = (mGuid.packed.l0 == guid.mGuid.packed.l0);
   bool ret_val1 = (mGuid.packed.l1 == guid.mGuid.packed.l1);
   bool ret_val2 = (mGuid.packed.l2 == guid.mGuid.packed.l2);
   bool ret_val3 = (mGuid.packed.l3 == guid.mGuid.packed.l3);
   return (ret_val0 && ret_val1 && ret_val2 && ret_val3);
   */

   return ( (mGuid.packed.l0 == guid.mGuid.packed.l0) &&
            (mGuid.packed.l1 == guid.mGuid.packed.l1) &&
            (mGuid.packed.l2 == guid.mGuid.packed.l2) &&
            (mGuid.packed.l3 == guid.mGuid.packed.l3) );
}

// ============================================================================
// Private methods.
// ============================================================================

GUID::GUID ()
   :  mGuid( GUID::NullGUID.mGuid )     // Assign a null guid
{
   ; /* Do nothing */
}

GUID::GUID (const struct vpr::GUID::StdGUID& guid)
{
   memcpy(&mGuid, &guid, sizeof(vpr::GUID::StdGUID));
}

GUID::GUID (const GUID& ns_guid, const std::string& name)
{
   generate(ns_guid,name);
}

void GUID::generate()
{
   uuid_create( (uuid_t*)(&mGuid.standard));
}

void GUID::generate(const GUID& ns_guid, const std::string& name)
{
   uuid_t temp_ns_id = *((uuid_t*)(&ns_guid.mGuid.standard));    // nasty, but works

   uuid_create_from_name((uuid_t*)(&mGuid.standard),
                         temp_ns_id,
                         (void*) name.c_str(), name.length());
}

void GUID::fromString (const std::string& guid_string)
{
   vpr::Uint32 m3[8];

   // The array of vpr::Uint32's is used to avoid alignment problems on
   // architectures such as MIPS.
   sscanf(guid_string.c_str(),
          "%08x-%04hx-%04hx-%02x%02x-%02x%02x%02x%02x%02x%02x",
          &mGuid.moz.m0, &mGuid.moz.m1, &mGuid.moz.m2,
          &m3[0], &m3[1], &m3[2], &m3[3], &m3[4], &m3[5], &m3[6], &m3[7]);

   // Fill the mGuid struct with the values read into m3[] above.
   for ( int i = 0; i < 8; i++ )
   {
      mGuid.moz.m3[i] = (vpr::Uint8) m3[i];
   }
}


} // End of vpr namespace
