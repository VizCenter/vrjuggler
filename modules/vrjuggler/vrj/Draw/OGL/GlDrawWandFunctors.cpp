/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2005 by Iowa State University
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

#include <vrj/vrjConfig.h>

#include <boost/concept_check.hpp>

#include <vrj/Draw/OGL/GlDrawWandFunctors.h>


namespace vrj
{

GlDrawConeWandFunctor::GlDrawConeWandFunctor()
   : mQuadObj(gluNewQuadric())
{
}

void GlDrawConeWandFunctor::draw(vrj::User* user)
{
   boost::ignore_unused_variable_warning(user);

   const float base = 0.2f;
   const float height = 0.6f;
   const int slices = 6;
   const int stacks = 1;
   
   glColor3f(0.0f, 1.0f, 0.0f);
   gluQuadricDrawStyle(mQuadObj, (GLenum) GLU_FILL);
   gluQuadricNormals(mQuadObj, (GLenum) GLU_SMOOTH);
   gluCylinder(mQuadObj, base, 0.0, height, slices, stacks);
}

void GlDrawRightAngleWandFunctor::draw(vrj::User* user)
{
   boost::ignore_unused_variable_warning(user);

   static GLfloat VertexData[] = {
      0.0140000f, -0.140000f, 0.0140000f, 0.0140000f, 0.028000f, 0.0140000f, -0.0140000f, 0.028000f, 0.0140000f, 
      -0.0140000f, -0.140000f, 0.0140000f, 0.0140000f, -0.140000f, -0.0140000f, 
      0.0140000f, 0.00000f, -0.0140000f, 0.0140000f, 0.028000f, 0.0140000f, 
      0.0140000f, -0.140000f, 0.0140000f, -0.0140000f, -0.140000f, -0.0140000f, 
      -0.0140000f, 0.00000f, -0.0140000f, 0.0140000f, 0.00000f, -0.0140000f, 
      0.0140000f, -0.140000f, -0.0140000f, -0.0140000f, -0.140000f, 0.0140000f, 
      -0.0140000f, 0.028000f, 0.0140000f, -0.0140000f, 0.00000f, -0.0140000f, 
      -0.0140000f, -0.140000f, -0.0140000f, -0.0140000f, -0.140000f, -0.0140000f, 
      0.0140000f, -0.140000f, -0.0140000f, 0.0140000f, -0.140000f, 0.0140000f, 
      -0.0140000f, -0.140000f, 0.0140000f, -0.0140000f, 0.028000f, 0.0140000f, 
      0.0140000f, 0.028000f, 0.0140000f, 0.028000f, 0.056000f, -0.042000f, 
      -0.028000f, 0.056000f, -0.042000f, 0.028000f, 0.042000f, -0.042000f, 
      -0.028000f, 0.042000f, -0.042000f, -0.028000f, 0.056000f, -0.042000f, 
      0.028000f, 0.056000f, -0.042000f, 0.0140000f, 0.028000f, 0.0140000f, 
      0.0140000f, 0.00000f, -0.0140000f, 0.028000f, 0.042000f, -0.042000f, 
      0.0140000f, 0.028000f, 0.0140000f, 0.028000f, 0.042000f, -0.042000f, 
      0.028000f, 0.056000f, -0.042000f, -0.0140000f, 0.028000f, 0.0140000f, 
      -0.028000f, 0.056000f, -0.042000f, -0.028000f, 0.042000f, -0.042000f, 
      -0.028000f, 0.042000f, -0.042000f, -0.0140000f, 0.00000f, -0.0140000f, 
      -0.0140000f, 0.028000f, 0.0140000f, 0.028000f, 0.042000f, -0.042000f, 
      0.0140000f, 0.00000f, -0.0140000f, -0.0140000f, 0.00000f, -0.0140000f, 
      -0.028000f, 0.042000f, -0.042000f
   };
   static GLint Indices[] = {
      0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 8, 9, 10, 8, 
      10, 11, 12, 13, 14, 12, 14, 15, 16, 17, 18, 16, 18, 19, 20, 21, 
      22, 20, 22, 23, 24, 25, 26, 24, 26, 27, 28, 29, 30, 31, 32, 33, 
      34, 35, 36, 37, 38, 39, 40, 41, 42, 40, 42, 43
   };

   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(3, GL_FLOAT, 0, VertexData);

   glColor3f(0.00000f, 1.00000f, 0.00000f);
   glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, &Indices[0]);
   glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, &Indices[3]);
   glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, &Indices[6]);
   glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, &Indices[9]);
   glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, &Indices[12]);
   glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, &Indices[15]);
   glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, &Indices[18]);
   glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, &Indices[21]);
   glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, &Indices[24]);
   glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, &Indices[27]);
   glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, &Indices[30]);
   glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, &Indices[33]);
   glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, &Indices[36]);
   glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, &Indices[39]);
   glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, &Indices[42]);
   glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, &Indices[45]);
   glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, &Indices[48]);
   glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, &Indices[51]);
   glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, &Indices[54]);
   glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, &Indices[57]);
}

}
