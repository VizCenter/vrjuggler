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

#include <vrj/vrjConfig.h>

#include <OsgNav.h>

#include <gmtl/Vec.h>
#include <gmtl/Coord.h>
#include <gmtl/Xforms.h>




//----------------------------------------------
//  Draw the scene.  A box on the end of the wand
//----------------------------------------------

void OsgNav::preFrame()
{

//vprDEBUG(vprDBG_ALL,0) << "------- preFrame ------\n" << vprDEBUG_FLUSH;
/*
cout  << "Wand Buttons:"
                      << " 0:" << mButton0->getData()
                      << " 1:" << mButton1->getData()
                      << " 2:" << mButton2->getData()
                      << " 3:" << mButton3->getData()
                      << " 4:" << mButton4->getData()
                      << " 5:" << mButton5->getData() << endl;
*/

    float inc = 0.005f;

    // MOVE the model around

   //If the model has moved +/- 10 move back
    //cout << posCount << endl;
    if((mPos[0] > 10.0f) || (mPos[0] < -10.0f))
   {
      posInc *= -1.0f;
   }

   mPos[0] += posInc;

   //Do the actual move
    osg::Matrix cur_xform = mModelTrans->getMatrix();
    cur_xform.setTrans(mPos);
    mModelTrans->setMatrix(cur_xform);

   // -- Get wand info -- //
    gmtl::Matrix44f* wandMatrix;
    wandMatrix = mWand->getData();      // Get the wand matrix

    osg::Matrix osgWandMat;
    //float * fPtr;
    osgWandMat.set(wandMatrix->getData());


   if(mButton0->getData() == gadget::Digital::ON)
    {
      //Move in the direction of the wand
        speed = speed + inc;
      cout << "speed: " << speed << std::endl;
    }
    if(mButton1->getData() == gadget::Digital::ON)
    {
        //joint->preRotate(5.0f, 0.0f, 0.0f, 1.0f);
      speed = 0;
    }
    if(mButton2->getData() == gadget::Digital::ON)
    {
        //joint->preRotate(-5.0f, 0.0f, 0.0f, 1.0f);
        speed = speed - inc;
      cout << "speed: " << speed << std::endl;
    }


    //Navigation
    gmtl::Vec3f direction;
    gmtl::Vec3f Zdir = gmtl::Vec3f(0.0f, 0.0f, speed);
    gmtl::xform(direction, *wandMatrix, Zdir);
    //mNavTrans->preTranslate(direction[0], direction[1], direction[2]);
     mNavTrans->preMult(osg::Matrix::translate(direction[0], direction[1], direction[2]));
}

void OsgNav::bufferPreDraw()
{
   glClearColor(0.0, 0.0, 0.0, 0.0);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OsgNav::myInit()
{
    //
   //          /-- mNoNav
   // mRootNode
    //         \-- mNavTrans -- mModelTrans -- mModel

   //The top level nodes of the tree
    mRootNode = new osg::Group();
    mNoNav   = new osg::Group();
    mNavTrans = new osg::MatrixTransform();

    mRootNode->addChild( mNoNav );
    mRootNode->addChild( mNavTrans );

   //Load the models and add them to the tree
   mModelTrans  = new osg::MatrixTransform();         // Transform node for the model

    //model = osgDB::readNodeFile("paraglider.osg");
    //mModel = osgDB::readNodeFile("dumptruck.osg");

   std::cout << "Attempting to load file: " << mFileToLoad << "... ";
   mModel = osgDB::readNodeFile(mFileToLoad);
   std::cout << "done." << std::endl;

   // Could use this to correct for incorrectly rotated models
   mModelBaseXform = new osg::MatrixTransform();
   //mModelBaseXform->preRotate(90.0f, 1.0f, 0.0f, 0.0f);
   mModelBaseXform->preMult( osg::Matrix::rotate(90.0f, 1.0f, 0.0f, 0.0f));
   mModelBaseXform->setMatrix( osg::Matrix::scale(50.0f, 50.0f, 50.0f));

   // Add model to the tree
   mModelTrans->addChild(mModel);
   mNavTrans->addChild( mModelTrans );
   
   // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize(mRootNode);

   // traverse the scene graph setting up all osg::GeoSet's so they will use
    // OpenGL display lists.
    //osgUtil::DisplayListVisitor dlv(osgUtil::DisplayListVisitor::SWITCH_ON_DISPLAY_LISTS);
    //mRootNode->accept(dlv);

    //The increment to move the model by
    mPos[0] = 0.0f;
    mPos[1] = 0.0f;
    mPos[2] = 0.0f;
    //posInc = 0.05f;
   posInc = 0.0f;

    speed = 0.0f;
}
