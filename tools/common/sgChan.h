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

#ifndef _SG_CHAN_H_
#define _SG_CHAN_H_
#pragma once

#include <sgInput.h>
#include <sgDrawFunctor.h>
#include <Math/vjMatrix.h>
#include <Math/vjVec3.h>
#include <GL/gl.h>
#include <GL/glu.h>

/**
 * sgChan: Defines a channel viewwindow into the set scene.
 *
 * - Keeps track of mouse input
 * - Tracks the viewport size
 * - Tracks the projection matrix
 * - Tracks near far settings
 * - Holds the draw functor object
 *
 */
class sgChan
{
public:
    sgChan()
    {
		viewMatrix.makeIdent();
		inputObject = NULL;
      mDrawFunc = NULL;
    }

    void setDrawFunc(sgDrawFunctor* func)
    { mDrawFunc = func; }
    const sgDrawFunctor* getDrawFunc()
    { return mDrawFunc; }
	
    void setViewport(int xo, int yo, int width, int height)
		{ vpXo = xo; vpYo = yo; vpWidth = width; vpHeight = height;}
    void getViewport(int* xo, int* yo, int* width, int* height)
		{ *xo = vpXo; *yo = vpYo; *width = vpWidth; *height = vpHeight; }

		/// Does the channel contain the given mouse coords
    int	containsMouse(int x, int y)
		{ return ((x >= vpXo) && (y >= vpYo) && (x <= (vpXo+vpWidth)) && (y <= (vpYo+vpHeight)));}

		/// Set the sgInput object to update
    void setInputObject(sgInput* mouseInput)
		{ inputObject = mouseInput; }
    sgInput* getInputObject()
    { return inputObject; }

		// Created a perspective window with auto aspect ratio.
    void setFov(float horiz)
    {
		projType = PERSPECTIVE;
		perspAspect = (vpWidth/vpHeight);
		perspFovy = horiz;
    }

    void setPersp(float fovy, float aspect)
		{ projType = PERSPECTIVE; perspFovy = fovy; perspAspect = aspect;}

    void setOrtho(float left, float right, float bottom, float top)
    {
		projType = ORTHO;
		orthoLeft = left; orthoRight = right; orthoBottom = bottom; orthoTop = top;
    }

    void setNearFar(float _Near, float _Far)
		{ Near = _Near; Far = _Far;}
	
	GLdouble getNear()
		{ return Near; }

	GLdouble getFar()
		{ return Far; }

		/**
		 * Sets the eye position and the eye euler xyz rotation.
		 *
		 */
    void setView(vjVec3 eyePos, vjVec3 eyeRot)
    {		
      // Build T*R
      vjMatrix rot_mat;
      rot_mat.makeXYZEuler(eyeRot[0], eyeRot[1], eyeRot[2]);
      vjMatrix mat;
      mat.makeTrans(eyePos[0], eyePos[1], eyePos[2]);
      mat.postMult(rot_mat);
      setViewMatrix(mat);
    }

    void setViewMatrix(const vjMatrix& mat)
    {
		viewMatrix = mat;
    }

public:
		/**
		 * Draws the channel with the given projection and viewport setup.
		 */
    void draw()
    {
		glViewport(vpXo, vpYo, vpWidth, vpHeight);
		
		glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			
			if(projType == PERSPECTIVE)
			{
				gluPerspective(perspFovy, perspAspect, Near, Far);
			} else {
				glOrtho(orthoLeft, orthoRight, orthoBottom, orthoTop, Near, Far);
			}
			
			// Put in the camera transformation matrix
			glMultMatrixf(viewMatrix.getFloatPtr());
		glMatrixMode(GL_MODELVIEW);
		
			// If we have an input object to update
		if(inputObject != NULL)
			inputObject->update();
		
		if(mDrawFunc != NULL)
         mDrawFunc->draw(this);
      else
         cerr << "Channel does not have drawFunc set.\n";
    }

private:
    sgDrawFunctor*   mDrawFunc;

    enum { PERSPECTIVE = 0, ORTHO =1};
    		
    /// Viewport parameters
    int vpXo, vpYo, vpWidth, vpHeight;

	 // Viewport params
    int      projType;
    GLdouble perspFovy, perspAspect;
    GLdouble orthoLeft, orthoRight, orthoBottom, orthoTop;
    GLdouble Near;
	 GLdouble Far;
    vjMatrix viewMatrix;

    sgInput* inputObject;
};
#endif
