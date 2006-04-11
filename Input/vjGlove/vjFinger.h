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
 
#ifndef VJ_FINGER___
#define VJ_FINGER___

//  DIJ = Distal Interphalangeal Joint  --- vjFinger tip
//  PIJ = Proximal "              "     --- Middle joint
//  MPJ = Metacarpo "             "     --- closest to vjHand
//  ABDUCT = spread of vjFingers
class vjFinger
{
public:
   vjFinger( const float& dij = -15, const float& pij = -45, const float& mpj = -45, const float& abduct = 0 ) : mDij( dij ), mPij( pij ), mMpj( mpj ), mAbduct( abduct )
   {
   }
   
   inline const float& dij() const { return mDij; }
   inline const float& pij() const { return mPij; }
   inline const float& mpj() const { return mMpj; }
   inline const float& abduct() const { return mAbduct; }
   
   inline void setDij( const float& dij ) { mDij = dij; }
   inline void setPij( const float& pij ) { mPij = pij; }
   inline void setMpj( const float& mpj ) { mMpj = mpj; }
   inline void setAbduct( const float& abduct ) { mAbduct = abduct; }
   
   inline void set( const float& dij = -15, const float& pij = -45, const float& mpj = -45, const float& abduct = 0 )
   {
       mDij = dij;
       mPij = pij;
       mMpj = mpj;
       mAbduct = abduct;
   }   
   
private: 
   float mDij, mPij, mMpj, mAbduct;
};


#endif
