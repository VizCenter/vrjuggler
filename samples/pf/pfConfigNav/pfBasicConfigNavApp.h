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

#ifndef _PF_BASIC_CONFIG_NAV_
#define _PF_BASIC_CONFIG_NAV_

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
#include <pfFileIO.h>

// nav includes
#include <pfNavDCS.h>
#include <velocityNav.h>
#include <collider.h>
#include <planeCollider.h>
#include <pfPogoCollider.h>
#include <pfRayCollider.h>
#include <pfBoxCollider.h>

#include <simplePfNavApp.h>

// ----- pfBasicConfigNav class ---- //
// Declare my application class
class pfBasicConfigNavApp : public simplePfNavApp
{
public:
   pfBasicConfigNavApp( )
   {
     mCurrentConfig = NULL;
   }

   bool configApp(vjConfigChunk* chunk);

public:     // RECONFIG STUFF
   virtual bool configCanHandle(vjConfigChunk* chunk)
   {
      return configCanHandleChunk(chunk);
   }

   // This function is so that others can query this object to
   // see if it can be configured with the given information
   static bool configCanHandleChunk(vjConfigChunk* chunk)
   {
      std::string chunk_type = (std::string)chunk->getType();

      if(std::string("pf_basic_nav_app") == chunk_type)
         return true;
      else
         return false;
   }


   //: Are any application dependencies satisfied
   // If the application requires anything special of the system for successful
   // initialization, check it here.
   // If retval == false, then the application will not be started yet
   //    retval == true, application will be allowed to enter the system
   virtual bool depSatisfied()
   { return true; }

protected:
   //! NOTE: Inherited from vjConfigChunkHandler
   virtual bool configAdd(vjConfigChunk* chunk)
   {
      configApp(chunk);
      return true;
   }

   //! NOTE: INherited from vjConfigChunkHandler
   virtual bool configRemove(vjConfigChunk* chunk)
   { vjASSERT(false); return false; }

protected:
   vjConfigChunk* mCurrentConfig;      // config chunk that is current configuring this thingie
};

// Get the properties from the chunks
// XXX: Smart update
// Set the properties
// Load with new settings
bool pfBasicConfigNavApp::configApp(vjConfigChunk* chunk)
{
   int x;

   vjASSERT(std::string("pf_basic_nav_app") == (std::string)chunk->getType());

   // Set the properties based on the chunk values
   std::string app_name = chunk->getProperty("Name");
   std::string file_path = chunk->getProperty("file_path");
   vjVec3 initial_pos;
   initial_pos.set((float)chunk->getProperty("start_location",0),
                   (float)chunk->getProperty("start_location",1),
                   (float)chunk->getProperty("start_location",2));

   vjDEBUG_BEGIN(vjDBG_ALL,0) << "pfBasicConfigNav::configApp: " << app_name << "===========================\n" << vjDEBUG_FLUSH;

   // models
   mModelList.clear();//start out clean
   for (x = 0; x < chunk->getNum( "Model" ); ++x)
   {
      vjConfigChunk* model_chunk = chunk->getProperty( "Model", x );
      Model m;
      m.description = (std::string)model_chunk->getProperty( "Name" );
      m.filename = (std::string)model_chunk->getProperty( "filename" );
      vjDEBUG_NEXT(vjDBG_ALL,0) << "Reading " <<m.filename<<" model from the config file\n"<< vjDEBUG_FLUSH;;
      m.scale = (float)model_chunk->getProperty( "Scale" );
      m.pos.set( (float)model_chunk->getProperty( "x" ),
            (float)model_chunk->getProperty( "y" ),
            (float)model_chunk->getProperty( "z" ) );
      m.rot.set( (float)model_chunk->getProperty( "rotx" ),
            (float)model_chunk->getProperty( "roty" ),
            (float)model_chunk->getProperty( "rotz" ) );
      m.isCollidable = (bool)model_chunk->getProperty( "collidable" );
      addModel( m );
   }

   // sounds
   mSoundList.clear();//start out clean
   for (x = 0; x < chunk->getNum( "Sound" ); ++x)
   {
      vjConfigChunk* sound_chunk = chunk->getProperty( "Sound", x );
      Sound s;
      s.name = (std::string)sound_chunk->getProperty( "Name" );
      s.alias = (std::string)sound_chunk->getProperty( "soundAlias" );
      vjDEBUG_NEXT(vjDBG_ALL,0) <<"Reading "<<s.alias<<" sound from the config file\n"<< vjDEBUG_FLUSH;
      s.positional = (bool)sound_chunk->getProperty( "positional" );
      s.pos.set( (float)sound_chunk->getProperty( "x" ),
            (float)sound_chunk->getProperty( "y" ),
            (float)sound_chunk->getProperty( "z" ) );
      mSoundList.push_back( s );
   }

   setFilePath( file_path );
   setInitialNavPos( initial_pos );
   mBoundingSize = chunk->getProperty("bounding_size");

   vjDEBUG_NEXT(vjDBG_ALL,0) << "   filepath: " << file_path << std::endl
                             << vjDEBUG_FLUSH;
   vjDEBUG_NEXT(vjDBG_ALL,0) << "initial pos: " << initial_pos << std::endl
                             << vjDEBUG_FLUSH;
   vjDEBUG_NEXT(vjDBG_ALL,0) << " bound size: " << mBoundingSize << std::endl
                             << vjDEBUG_FLUSH;
   vjDEBUG_END(vjDBG_ALL,0) << "========================================\n" << vjDEBUG_FLUSH;

   // Initialize the models and sounds
   initializeModels();                 // Only actally gets processed if initScene has already run
   initializeSounds();                 // Only actally gets processed if initScene has already run

   return true;
}


#endif
