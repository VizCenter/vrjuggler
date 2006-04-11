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
 * -----------------------------------------------------------------
 * File:          $RCSfile$
 * Date modified: $Date$
 * Version:       $Revision$
 * -----------------------------------------------------------------
 *
 *************** <auto-copyright.pl END do not edit this line> ***************/

#ifndef _VRJ_OSG_APP_H_
#define _VRJ_OSG_APP_H_

#include <vrj/vrjConfig.h>

#include <vrj/Draw/OGL/GlApp.h>
#include <vrj/Draw/OGL/GlContextData.h>

#include <vrj/Display/CameraProjection.h>

#include <osg/Vec3>
#include <osg/Matrix>
#include <osg/Transform>
#include <osg/Group>

#include <osgUtil/SceneView>


namespace vrj
{

/**
 * OsgApp: Encapulates an open scene graph application.
 *
 * PURPOSE:
 *
 * See also: GlApp
 *
 */
class VJ_CLASS_API OsgApp : public GlApp
{
public:
   OsgApp(Kernel* kern) : GlApp(kern)
   {;}

   virtual ~OsgApp() {;}

   virtual void initScene() = 0;
   virtual osg::Group* getScene() = 0;

   /**
    * Configures newly created scene viewers.
    * This is called immediately after a new scene viewer is created for a
    * context.  This is the place to configure application background colors
    * and other viewer specific information.
    */
   virtual void configSceneView(osgUtil::SceneView* newSceneViewer)
   {
      newSceneViewer->setDefaults();
      newSceneViewer->setBackgroundColor( osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f) );
   }

   /**
    * Function to draw the scene.
    * Override this function with the user draw routine.
    *
    * @pre OpenGL state has correct transformation and buffer selected.
    * @post The current scene has been drawn.
    */
   virtual void draw();

   // Initialize
   // Make sure to call initScene if you override this function
   virtual void init()
   {
      //Create the scene
      this->initScene();
   }

   /**
    * Function that is called immediately after a new context is created.
    * Use this function to create context specific data structures.
    * i.e. Display lists, Texture objects, etc.
    *
    * @pre The ogl context has been set to the new context.
    * @post Application has completed in initialization the user wishes.
    */
   virtual void contextInit();

   /**
    * Function that is called immediately before a context is closed.
    * Use the function to clean up any context data structures.
    */
   virtual void contextClose()
   {;}

   /**
    * Function that is called upon entry into the context for a draw.
    *
    * @pre The ogl context has been set to the context for drawing.
    * @post User application has executed any commands that need
    *       to only be executed once per context, per frame.
    * @note This function can be used for things that need to happen
    *        every frame, but only once per context.
    *   <br> Ex: Dynamically Create display lists
    */
   virtual void contextPreDraw()
   {;}

   /**
    * Function that is called upon entry into a buffer of a GL context.
    *
    * @pre The ogl context has been set to the context for drawing.
    * @post User application has executed any commands that need
    *       to only be executed once per context, per buffer, per frame.
    * @note This function is designed to be used when you want to do something
    *        only once per buffer (ie.once for left buffer, once for right
    *        buffer).
    *   <br> Ex: glClear's need to be done in this method
    */
   virtual void bufferPreDraw()
   {;}

   /**
    * Function that is called at the beginning of the drawing of each pipe.
    *
    * @pre The library is preparing to render all windows on a given pipe.
    * @post Any pre-pipe user calls have been done.
    * @note Currently the OGL context is not set when this function is called.
    *       This is a TEST function.  USE AT YOUR OWN RISK!!!
    */
   virtual void pipePreDraw()
   {;}

protected:
   vrj::GlContextData< osgUtil::SceneView* > sceneViewer;
};

inline void OsgApp::contextInit()
{
   unsigned int unique_context_id = GlDrawManager::instance()->getCurrentContext();

   // --- Create new context specific scene viewer -- //
   osgUtil::SceneView* new_sv = new osgUtil::SceneView;
   this->configSceneView(new_sv);            // Configure the new viewer
   new_sv->getState()->setContextID(unique_context_id);
   (*sceneViewer) = new_sv;
}


inline void OsgApp::draw()
{
   osgUtil::SceneView* sv(NULL);
   sv = (*sceneViewer);    // Get context specific scene viewer
   vprASSERT( sv != NULL);

   // Add the tree to the scene viewer
   sv->setSceneData(getScene());
   sv->setCalcNearFar(false);

   //Take care of the view port
   GLint view[4];
   glGetIntegerv(GL_VIEWPORT, view);      //Get the view port that juggler sets
   sv->setViewport(view[0],view[1],view[2],view[3]);

   //Get the view matrix and the frustrum form the draw manager
   GlDrawManager* drawMan = dynamic_cast<GlDrawManager*> ( this->getDrawManager() );
   vprASSERT(drawMan != NULL);
   GlUserData* userData = drawMan->currentUserData();

   // Configure the viewport information
   //vjViewport* cur_vp = userData->getViewport();
   //float xo, yo, xs, ys;
   //cur_vp->getOriginAndSize(xo,yo,xs,ys);
   //sv->setViewport(xo, yo, xs, ys);

   // Copy the matrix
   Projection* project = userData->getProjection();
   float* vj_proj_view_mat = project->mViewMat.mData;
   osg::Matrix osg_proj_xform_mat;
   osg_proj_xform_mat.set( vj_proj_view_mat );

   //Get the frustrum
   Frustum frustum = project->mFrustum;

   //Reset the camera
   osg::Camera* the_cam = sv->getCamera();

   // Setup the camera
   the_cam->home();
   the_cam->setAdjustAspectRatioMode(osg::Camera::ADJUST_NONE);      // Tell it not to adjust the aspect ratio at all

   //Set the frustrum (this is set with the matrix below)
   //float near_val = frustum[Frustum::VJ_NEAR];
   the_cam->setFrustum(frustum[Frustum::VJ_LEFT],    frustum[Frustum::VJ_RIGHT],
                       frustum[Frustum::VJ_BOTTOM],  frustum[Frustum::VJ_TOP],
                       frustum[Frustum::VJ_NEAR],    frustum[Frustum::VJ_FAR]);

   //Set the look at
   // NOTE: This is on the wrong stack !!!!
   the_cam->attachTransform(osg::Camera::MODEL_TO_EYE, &osg_proj_xform_mat);

   //Draw the scene
   sv->app();
   sv->cull();
   sv->draw();
}


};

#endif
