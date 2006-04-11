/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998, 1999, 2000, 2001, 2002
 *   by Iowa State University
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

#include <iostream>

#include <Math/vjPlane.h>
#include <Math/vjVec3.h>
#include <Math/vjSeg.h>

// Program to test the functionality of the vjSeg class
//
//
int main(void)
{
   // Create some planes and segs to use in the tests
   const vjVec3 x_axis(1,0,0);
   const vjVec3 y_axis(0,1,0);
   const vjVec3 z_axis(0,0,1);
   const vjVec3 origin(0,0,0);
   vjPlane zy_plane; zy_plane.makePts(z_axis,origin,y_axis);

   // Test finding collision with a triangle
   vjSeg seg_through, seg_parallel, seg_miss;
   float t_dist;
   bool hit;

   seg_through.makePts(vjVec3(1,0.25,0.25), vjVec3(-1,0.25,0.25) );    // Passes through the plane at the origin
   seg_parallel.makePts(x_axis, (x_axis+z_axis));                 // Is parallel to the plane
   seg_miss.makePts(vjVec3(1,1,1), vjVec3(-1,0.5,0.5));  // Misses

   hit = seg_through.isectTriangle(z_axis,origin,y_axis,&t_dist);
   std::cout << "Testing isectTriangle (through): ";
   if(hit && VJ_IS_ZERO(t_dist - 1.0f))
      std::cout << "Passed.\n";
   else
      std::cout << "FAILED!!!!\n";

   hit = seg_parallel.isectTriangle(z_axis,origin,y_axis,&t_dist);
   std::cout << "Testing isectTriangle (parallel): ";
   if(!hit)
      std::cout << "Passed.\n";
   else
      std::cout << "FAILED!!!!\n";

   hit = seg_miss.isectTriangle(z_axis,origin,y_axis,&t_dist);
   std::cout << "Testing isectTriangle (miss): ";
   if(!hit)
      std::cout << "Passed.\n";
   else
      std::cout << "FAILED!!!!\n";



}
