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

#ifndef _VPR_GUID_H_
#define _VPR_GUID_H_

#include <vpr/vprConfig.h>

#include <string>
#include <cctype>
//#include <uuid/sysdep.h>
//#include <uuid/uuid.h>

#include <vpr/vprTypes.h>
#include <vpr/Util/Assert.h>
#include <boost/concept_check.hpp>  // for ignore_unused_variable_warning


namespace vpr
{

class GUIDFactory;

class VPR_CLASS_API GUID
{
public:
   /** Tag to the constructor to force generation: dso::GUID guid(dso::GUID::generate_tag);
*/
   class GenerateTag { };
   static GenerateTag generateTag;

public:
   ~GUID (void) {;}

   GUID(const GenerateTag tag)
   {
      boost::ignore_unused_variable_warning(tag);
      generate();
   }

   /**
    * Converts this GUID to its corresponding string representation.
    */
   std::string toString(void) const;

   bool operator==(const GUID& guid_obj) const;

   bool operator!= (const GUID& guid_obj) const
   {
      return ! (*this == guid_obj);
   }

   bool operator<(const GUID& r) const
   {
      bool ret_val(false);

      if (mGuid.packed.l0 < r.mGuid.packed.l0)
      {  ret_val = true; }
      else if(mGuid.packed.l0 == r.mGuid.packed.l0)
      {
         if(mGuid.packed.l1 < r.mGuid.packed.l1)
         {  ret_val = true; }
         else if(mGuid.packed.l1 == r.mGuid.packed.l1)
         {
            if(mGuid.packed.l2 < r.mGuid.packed.l2)
            {  ret_val = true; }
            else if(mGuid.packed.l2 == r.mGuid.packed.l2)
            {
               if(mGuid.packed.l3 < r.mGuid.packed.l3)
               {
                  ret_val = true;
               }
            }
         }
      }

      return ret_val;
   }

   struct StdGUID
   {
      vpr::Uint32 m0;
      vpr::Uint16 m1;
      vpr::Uint16 m2;
      vpr::Uint8  m3;
      vpr::Uint8  m4;
      vpr::Uint8  m5[6];
   };

public:
   /**
    * Generates empty guid - Sets equal to GUID::NullGUID
    */
   GUID();

   /**
    * Generates a GUID from the given struct.
    */
   GUID(const struct StdGUID& guid);

   /**
    * Generates a GUID from the given string representation of the GUID using
    * a char*.
    * Format: "8x-4x-4x-2x2x-2x2x2x2x2x2x"
    * @param   guid_string    Ptr to a string that is used to inialize guid. Must be non-NULL
    */
   GUID (const char* guid_string)
   {
      vprASSERT( (guid_string != NULL) && "Tried to initialize with NULL ptr");
      vprASSERT( (isalpha(*guid_string) || isdigit(*guid_string)) && "Possibly invalid pointer passed to constructor");
      if(NULL != guid_string)
      {
         std::string temp(guid_string);
         fromString(temp);
      }
   }

   /**
    * Generates a GUID from the given string representation of the GUID using
    * a std::string.
    * Format: "8x-4x-4x-2x2x-2x2x2x2x2x2x"
    */
   GUID (const std::string& guid_string)
   {
      fromString(guid_string);
   }

   /**
    * Generates a GUID based on the given name that is part of the namespace
    * identified by the given namespace GUID.
    */
   GUID(const GUID& ns_guid, const std::string& name);

   GUID (const GUID& obj)
   {
      mGuid.packed.l0 = obj.mGuid.packed.l0;
      mGuid.packed.l1 = obj.mGuid.packed.l1;
      mGuid.packed.l2 = obj.mGuid.packed.l2;
      mGuid.packed.l3 = obj.mGuid.packed.l3;
   }



   GUID& operator= (const GUID& obj)
   {
      if(&obj == this) // Check for self
         return *this;

      mGuid.packed.l0 = obj.mGuid.packed.l0;
      mGuid.packed.l1 = obj.mGuid.packed.l1;
      mGuid.packed.l2 = obj.mGuid.packed.l2;
      mGuid.packed.l3 = obj.mGuid.packed.l3;
      return *this;
   }

   void generate();
   void generate(const GUID& ns_guid, const std::string& name);

   union _vpr_guid
   {
      //uuid_t leach;

      struct _moz
      {
         vpr::Uint32 m0;
         vpr::Uint16 m1;
         vpr::Uint16 m2;
         vpr::Uint8 m3[8];
      } moz;

      struct StdGUID standard;

      struct _packed
      {
         vpr::Uint32 l0;
         vpr::Uint32 l1;
         vpr::Uint32 l2;
         vpr::Uint32 l3;
      } packed;
   } mGuid;

   friend class vpr::GUIDFactory;

   static const vpr::GUID NullGUID;

   /**
    * Hasher for vpr::GUID.  This can be used with std::hash_map and
    * friends.
    */
   struct hash
   {
      vpr::Uint32 operator() (const vpr::GUID& guid) const
      {
         return guid.mGuid.packed.l0 + guid.mGuid.packed.l1 + guid.mGuid.packed.l2 + guid.mGuid.packed.l3;
      }
   };

private:

   /**
    * Performs the real work of generating a GUID from a string.
    * Format: "8x-4x-4x-2x2x-2x2x2x2x2x2x"
    */
   void fromString(const std::string& guid_string);
};

} // End of vpr namespace

namespace std {

inline std::ostream& operator<<(std::ostream& out, const vpr::GUID& guid)
{
   out << guid.toString();
   return out;
}

}


#endif /* _VPR_GUID_H_ */
