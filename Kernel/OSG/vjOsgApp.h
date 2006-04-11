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

#ifndef _VJ_OSG_APP_
#define _VJ_OSG_APP_

#include <vjConfig.h>

#include <Kernel/GL/vjGlApp.h>

#include <osg/Vec3>
#include <osg/Matrix>
#include <osg/Transform>
#include <osg/Group>

#include <osgUtil/SceneView>


//-----------------------------------------------------------
//: vjOsgApp: Encapulates an open scene graph application.
//
// PURPOSE:
//
// See also: vjGlApp
//
//-------------------------------------------------------------
//!PUBLIC_API:
class  vjOsgApp : public vjGlApp
{
public:
   vjOsgApp(vjKernel* kern) : vjGlApp(kern)
   {;}

   virtual ~vjOsgApp() {;}

   virtual void initScene() = 0;
   virtual osg::Group* getScene() = 0;

   //: Function to draw the scene
   // Override this function with the user draw routine
   //!PRE: OpenGL state has correct transformation and buffer selected
   //!POST: The current scene has been drawn
   virtual void draw();

   // Initialize
   // Make sure to call initScene if you override this function
   virtual void init()
   {
      //Create the scene
   	sceneView = new osgUtil::SceneView;
	   sceneView->setDefaults();

      initScene();
   }

   //: Function that is called immediately after a new context is created
   //  Use this function to create context specific data structures.
   //  i.e. Display lists, Texture objects, etc.
   //! PRE: The ogl context has been set to the new context
   //! POST: Application has completed in initialization the user wishes
   virtual void contextInit()
   {;}

   //: Function that is called immediately before a context is closed
   // Use the function to clean up any context data structures
   virtual void contextClose()
   {;}

   //: Function that is called upon entry into the context for a draw
   //! PRE: The ogl context has been set to the context for drawing
   //! POST: User application has executed any commands that need
   //+   to only be executed once per context, per frame
   //! NOTE: This function can be used for things that need to happen
   //+       every frame, but only once per context
   //+  <br> Ex: Dynamically Create display lists
   virtual void contextPreDraw()
   {;}

   //: Function that is called upon entry into a buffer of a gl context
   //! PRE: The ogl context has been set to the context for drawing
   //! POST: User application has executed any commands that need
   //+   to only be executed once per context, per buffer, per frame
   //! NOTE: This function is designed to be used when you want to do something
   //+       only once per buffer (ie.once for left buffer, once for right buffer)
   //+  <br> Ex: glClear's need to be done in this method
   virtual void bufferPreDraw()
   {;}

   //: Function that is called at the beginning of the drawing of each pipe
   //!PRE: The library is preparing to render all windows on a given pipe
   //!POST: Any pre-pipe user calls have been done
   //!NOTE: Currently the OGL context is not set when this function is called <br>
   //+      This is a TEST function.  USE AT YOUR OWN RISK!!!
   virtual void pipePreDraw()
   {;}

protected:
   osg::ref_ptr<osgUtil::SceneView> sceneView;        // Renderer.  XXX: Probably need multiple of them
};


inline void vjOsgApp::draw()
{
   // Add the tree to the scene viewer
	sceneView->setSceneData(getScene());
	sceneView->setCalcNearFar(false);

   //Take care of the view port (this does not work)
	GLint view[4];
	glGetIntegerv(GL_VIEWPORT, view);      //Get the view port that juggler sets
	sceneView->setViewport(view[0],view[1],view[2],view[3]);

   //Get the view matrix and the frustrum form the draw manager
   vjGlDrawManager* drawMan = dynamic_cast<vjGlDrawManager*> ( this->getDrawManager() );
   vjASSERT(drawMan != NULL);
   vjGlUserData* userData = drawMan->currentUserData();
	
   // Configure the viewport information
   //vjViewport* cur_vp = userData->getViewport();
   //float xo, yo, xs, ys;
   //cur_vp->getOriginAndSize(xo,yo,xs,ys);
   //sceneView->setViewport(xo, yo, xs, ys);

   // Copy the matrix
   vjProjection* project = userData->getProjection();
   float* vj_proj_view_mat = project->mViewMat.getFloatPtr();
   osg::Matrix osgMat;
   osgMat.set(	vj_proj_view_mat );

   //Get the frustrum
   vjFrustum frustum = project->mFrustum;

   // Get the viewport
   vjViewport* vp = userData->getViewport();
	 
   //Reset the camera
   osg::Camera* the_cam = sceneView->getCamera();
   vjCameraProjection* sim_cam_proj(NULL);   

   switch(vp->getType())
   {
   case vjViewport::SURFACE:
      the_cam->home();
      the_cam->setAdjustAspectRatioMode(osg::Camera::ADJUST_NONE);      // Tell it not to adjust the aspect ratio at all
   
      //Set the frustrum (this is set with the matrix below)
      //float near_val = frustum[Frustum::VJ_NEAR];
      the_cam->setFrustum(frustum[Frustum::VJ_LEFT],   frustum[Frustum::VJ_RIGHT],
                          frustum[Frustum::VJ_BOTTOM],  frustum[Frustum::VJ_TOP],
                          frustum[Frustum::VJ_NEAR],             frustum[Frustum::VJ_FAR]);
      break;

   case vjViewport::SIM:
      sim_cam_proj = dynamic_cast<vjCameraProjection*>(project);
      vprASSERT(sim_cam_proj != NULL && "Trying to use non-camera projection for simulator");
      the_cam->home();
      the_cam->setAdjustAspectRatioMode(osg::Camera::ADJUST_HORIZONTAL);
      //chan->setNearFar(frustum[Frustum::VJ_NEAR], frustum[Frustum::VJ_FAR]);
      //chan->setFOV(0.0f, cam_proj->mVertFOV);
      the_cam->setPerspective( sim_cam_proj->mVertFOV, sim_cam_proj->mAspectRatio,
                               frustum[Frustum::VJ_NEAR], frustum[Frustum::VJ_FAR]);
      break;

   default:
      vprASSERT(false);
      break;
   }

   //Set the look at
   // NOTE: This is on the wrong stack !!!!
   the_cam->attachTransform(osg::Camera::MODEL_TO_EYE, &osgMat);
		
   //Draw the scene
   sceneView->app();
   sceneView->cull();
   sceneView->draw();
}

#endif

