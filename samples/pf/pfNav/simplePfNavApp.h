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

#include <vjPfAppStats.h> // for stats
#include <Kernel/Pf/vjPfApp.h>    // the performer application base type
class pfNavDCS;
class navigator;

extern int AppNotifyPreTrav( pfTraverser* trav, void* data );
extern int AppNotifyPostTrav( pfTraverser* trav, void* data );

//: Ready-Made Application
//
// The ready-made-application-in-a-box-handy-dandy-automatic-do-all
// vr juggler performer application for sounds, and models.  Just
// tell it your models and sounds, and it does the rest.
// 
// Better than snake oil, this application easy to use, 
// but really hard to understand.
// - i.e. don't use this to learn vrjuggler or performer!
//        you've been warned...
class simplePfNavApp : public vjPfApp
{
public: 
   // Model and sound member classes
   class Model
   {
   public:
      Model();
      Model( const std::string& desc, const std::string& file_name,
            const float& s, const vjVec3& position, const vjVec3& rotation, 
            const bool& collidable );

      // Config parameters
      std::string description;
      std::string filename;
      float       scale;
      vjVec3      pos;
      vjVec3      rot;
      bool        isCollidable;

      // Run-time information
      // --- modelDCS -- modelNode --- (Model info) -- bok bok bok
      pfDCS*      modelDCS;
      pfNode*     modelNode;
   };

   class Sound
   {
    public:
      Sound();
      Sound( const std::string& sound_name, const std::string& alias_name, 
            const bool& isPositional, const vjVec3& position );
      
      std::string name;
      std::string alias;
      bool        positional;
      vjVec3      pos;
   };

// application callbacks:
public:
   //: default constructor
   simplePfNavApp();

   //: virtual destructor
   virtual ~simplePfNavApp();

   //: data init
   virtual void init();
   virtual void apiInit();
   virtual void preForkInit();

   //: Initialize the scene graph
   virtual void initScene();

   //: load the model into the scene graph
   // If another model is already in the scene graph, we destroy it and load the newly configured one.
   // The model loaded is based on the configuration information that we currently have
   // This may be called multiple times
   virtual void initializeModels();
   virtual void initializeSounds();

   //: Return the current scene graph
   virtual pfGroup* getScene();

   //: Function called in application process for each active channel each frame
   // Called immediately before draw (pfFrame())
   virtual void appChanFunc(pfChannel* chan);

   //: Function called by the DEFAULT drawChan function before clearing the channel
   // and drawing the next frame (pfFrame())
   virtual void preDrawChan(pfChannel* chan, void* chandata);

   //: Function called after pfSync and before pfDraw
   virtual void preFrame();

   //: Function called after pfDraw
   virtual void intraFrame();

// Methods to configure the application
public:  
   //: Reset the application to initial state (removes all sounds and models)
   virtual void reset();

   //: set to true to enable navigation
   void enableNav( bool state = true );

   //: These must be set before the kernel starts calling the application
   void addModelFile( const std::string& filename );

   // Add a model to the application
   void addModel( const Model& m );

   // add a sound to the application
   void addSound( const Sound& s );

   void addFilePath( const std::string& path );
   void setFilePath( const std::string& path );
   void setInitialNavPos( const vjVec3& initialPos );

   // Go to the next navigator
   void cycleNavigator();

   void setNavigator( unsigned new_index );

   void enableStats();
   void disableStats();

// data: (should be private in the future.)
public:
   // models and sounds.
   std::vector< Model > mModelList;
   std::vector< Sound > mSoundList;

   bool           mDisableNav;

   // CONFIG PARAMS
   std::string    mFilePath;
   vjVec3         mInitialNavPos;
   float          mBoundingSize;       // XXX: This is a hack and should be refactored

   int            mStatusMessageEmitCount;

   bool           mUseStats;
   vjPfAppStats   mStats;

   // navigation objects.
   std::vector<navigator*>    mNavigators;      // A list of the navigators in the system
   unsigned                   mCurNavIndex;     // Index of the current navigator
   pfNavDCS*                  mNavigationDCS;
   vjDigitalInterface         mNavCycleButton;  // Button to cycle the navigation

   // SCENE GRAPH NODES
   pfGroup*       mLightGroup;
   pfGroup*       mNoNav;
   pfLightSource* mSun;                      // Sun to light the environment
   pfGroup*       mRootNode;                 // The root of the scene graph
   pfGroup*       mConfiguredCollideModels;               // Collidable model of the world
   pfGroup*       mConfiguredNoCollideModels;                 // UnCollidable model of the world
   pfGroup*       mSoundNodes;               // The sounds of the world
   pfGroup*       mCollidableModelGroup;     // Part of the world that is collidable
   pfGroup*       mUnCollidableModelGroup;   // Part of the world that is collidable
};

#endif

