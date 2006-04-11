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

#ifndef _SIMPLE_PF_NAV_APP_H_
#define _SIMPLE_PF_NAV_APP_H_

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <Performer/pf/pfLightSource.h>
#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfDCS.h>
#include <Performer/pfdu.h>
#include <Performer/pfutil.h>

// --- VR Juggler Stuff --- //
#include <Kernel/vjKernel.h>
#include <Kernel/Pf/vjPfApp.h>    // the performer application base type
#include <Kernel/vjDebug.h>
#include <Kernel/vjProjection.h>  // for setNearFar (for setting clipping planes)

// Declare my application class
class simplePfApp : public vjPfApp
{
public:
   simplePfApp() :
      mModelFileName(""), mLightGroup( NULL ), mSun(NULL), mRootNode( NULL ), mModelRoot( NULL )
   {
   }

   virtual ~simplePfApp (void)
   {;}

   // Called between pfInit and pfConfig
   // This function allows the user application to do any processing that needs
   // to happen before performer forks its processes off but after
   // pfInit()
   virtual void preForkInit()
   {
      // Initialize model converters
      pfdInitConverter( mModelFileName.c_str() );
   }

   // Initialize the scene graph
   // Called after pfInit & pfConfig but before apiInit
   virtual void initScene();

   /// Return the current scene graph
   virtual pfGroup* getScene()
   {
      return mRootNode;
   }

   // Set the name of the model to load
   // This must be called before the application is given to the kernel
   void setModel(std::string modelFile)
   { mModelFileName = modelFile; }

public:
   // CONFIG PARAMS
   std::string    mModelFileName;

   // SCENE GRAPH NODES
   pfGroup*       mLightGroup;
   pfLightSource* mSun;                      // Sun to light the environment
   pfGroup*       mRootNode;                 // The root of the scene graph
   pfNode*        mModelRoot;                // Root of the model
};

// ------- SCENE GRAPH ----
// a standard organized interface for derived applications:
//
//            /-- mLightGroup -- mSun
// mRootNode -- mModelRoot
//
void simplePfApp::initScene()
{
   // Load the scene
   vjDEBUG(vjDBG_ALL, 0) << "simplePfApp::initScene\n" << vjDEBUG_FLUSH;

   // Allocate all the nodes needed
   mRootNode             = new pfGroup;            // Root of our graph

   // Create the SUN light source
   mLightGroup = new pfGroup;
   mSun = new pfLightSource;
   mLightGroup->addChild( mSun );
   mSun->setPos( 0.3f, 0.0f, 0.3f, 0.0f );
   mSun->setColor( PFLT_DIFFUSE,1.0f,1.0f,1.0f );
   mSun->setColor( PFLT_AMBIENT,0.3f,0.3f,0.3f );
   mSun->setColor( PFLT_SPECULAR, 1.0f, 1.0f, 1.0f );
   mSun->on();

   // --- LOAD THE MODEL --- //
   mModelRoot = pfdLoadFile(mModelFileName.c_str());

   // --- CONSTRUCT STATIC Structure of SCENE GRAPH -- //
   mRootNode->addChild( mModelRoot );
   mRootNode->addChild(mLightGroup);
}

#endif

