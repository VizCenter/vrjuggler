/*
 * VRJuggler
 *   Copyright (C) 1997,1998,1999,2000
 *   Iowa State University Research Foundation, Inc.
 *   All Rights Reserved
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
 */

#include <simpleGloveApp.h>


void simpleGloveApp::myDraw()
{
	//cout<<"simpleGloveApp::myDraw()\n"<<flush;
   
   // Clear the scene
   glClearColor(0.0, 0.0, 0.0, 0.0);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);

   int finger;

      // -- Draw box on wand --- //
   vjMatrix finger_matrix;

   // Draw green balls on finger tips
   glColor3f(0.0f, 1.0f, 0.0f);
   glPushMatrix();
   {
      for(finger=vjGloveData::THUMB;finger<=vjGloveData::PINKY;finger++)
      {
      glPushMatrix();
         finger_matrix =
            mGlove->getPos((vjGloveData::vjGloveComponent)finger);
         glMultMatrixf(finger_matrix.getFloatPtr());
         drawSphere((0.1f*(1.0f/12.0f)), 4, 4);
      glPopMatrix();
      }
   }
   glPopMatrix();

   // Draw red rays out of fingers
   // I am not doing this in the most efficient way.
   // I am just trying to test the vector functions
   glColor3f(1.0f, 0.0f, 0.0f);
   glPushMatrix();
   {
      for(finger=vjGloveData::THUMB;finger<=vjGloveData::PINKY;finger++)
      {
      glPushMatrix();
         vjVec3   origin(0,0,0);    // Base of the vector
         finger_matrix =
            mGlove->getPos((vjGloveData::vjGloveComponent)finger);
         origin.xformFull(finger_matrix, origin);     // Go to new coord system
         vjVec3 end = origin + (0.25 * mGlove->getVector((vjGloveData::vjGloveComponent)finger));
         drawLine(origin, end);
      glPopMatrix();
      }
   }
   glPopMatrix();
}

void simpleGloveApp::initGLState()
{
   //cout<<"simpleGloveApp::initGLState()\n"<<flush;
   
   GLfloat light0_ambient[] = { .2,  .2,  .2,  1.0};
   GLfloat light0_diffuse[] = { 1.0,  1.0,  1.0,  1.0};
   GLfloat light0_specular[] = { 1.0,  1.0,  1.0,  1.0};
   GLfloat light0_position[] = {2000.0, 1000.0, 100.0, 1.0};
   GLfloat light0_direction[] = {-100, -100.0, -100.0};

   GLfloat mat_ambient[] = { 0.7, 0.7,  0.7,  1.0};
   GLfloat mat_diffuse[] = { 1.0,  0.5,  0.8,  1.0};
   GLfloat mat_specular[] = { 1.0,  1.0,  1.0,  1.0};
   GLfloat mat_shininess[] = { 50.0};
   GLfloat mat_emission[] = { 1.0,  1.0,  1.0,  1.0};
   GLfloat no_mat[] = { 0.0,  0.0,  0.0,  1.0};

   glLightfv(GL_LIGHT0, GL_AMBIENT,  light0_ambient);
   glLightfv(GL_LIGHT0, GL_DIFFUSE,  light0_diffuse);
   glLightfv(GL_LIGHT0, GL_SPECULAR,  light0_specular);
   glLightfv(GL_LIGHT0, GL_POSITION,  light0_position);
   glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, light0_direction);

   glMaterialfv( GL_FRONT, GL_AMBIENT, mat_ambient );
   glMaterialfv( GL_FRONT,  GL_DIFFUSE, mat_diffuse );
   glMaterialfv( GL_FRONT, GL_SPECULAR, mat_specular );
   glMaterialfv( GL_FRONT,  GL_SHININESS, mat_shininess );
   glMaterialfv( GL_FRONT,  GL_EMISSION, no_mat);

   glEnable(GL_DEPTH_TEST);
   glEnable(GL_NORMALIZE);
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glEnable(GL_COLOR_MATERIAL);
   glShadeModel(GL_SMOOTH);
}
