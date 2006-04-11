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


#ifndef _VJ_SIM_DISPLAY_H_
#define _VJ_SIM_DISPLAY_H_
//#pragma once

#include <vjConfig.h>
#include <Math/vjMatrix.h>
#include <Kernel/vjDebug.h>
#include <Input/InputManager/vjPosInterface.h>
#include <Kernel/vjDisplay.h>
#include <Kernel/vjCameraProjection.h>

class vjSimDisplay : public vjDisplay
{
public:
   vjSimDisplay() {
      mCameraProj = NULL;
   }

public:
   //: Configure the simulator
   virtual void config(vjConfigChunk* chunk)
   {
      vjDisplay::config(chunk);

      mType = SIM;
      mCameraProj = new vjCameraProjection;
      mCameraProj->config(chunk);

      std::string camera_proxy_str = chunk->getProperty("cameraPos");
      std::string wand_proxy_str = chunk->getProperty("wandPos");

      mCamera.init(camera_proxy_str);
      mWand.init(wand_proxy_str);      // Configure the wand to use

      if(mCamera.getProxyIndex() == -1)
      {
         vjDEBUG(vjDBG_ERROR,0) << clrOutNORM(clrRED,"ERROR:") << "vjSimDisplay:: Fatal Error: Camera not found named: " << camera_proxy_str.c_str() << endl;
         exit(1);
      }

      // Get drawing parameters
      mDrawProjections = chunk->getProperty("drawProjections");
      mSurfaceColor[0] = chunk->getProperty("surfaceColor", 0);
      mSurfaceColor[1] = chunk->getProperty("surfaceColor", 1);
      mSurfaceColor[2] = chunk->getProperty("surfaceColor", 2);
   }

   virtual void updateProjections()
   {
      updateInternalData();
      vjMatrix camera_pos = getCameraPos();
      mCameraProj->calcViewMatrix(camera_pos);
   }

   vjMatrix getCameraPos()
   { return mCameraPos; }

   vjMatrix getHeadPos()
   { return mHeadPos; }

   vjMatrix getWandPos()
   { return mWandPos; }

   vjProjection* getCameraProj()
   { return mCameraProj; }

public:  // Sim Drawing parameters
   bool shouldDrawProjections()
   { return mDrawProjections; }

   vjVec3 getSurfaceColor()
   { return mSurfaceColor; }

protected:
    //: Update internal simulator data
   void updateInternalData()
   {
      mHeadPos = *(mUser->getHeadPos());
      mWandPos = *(mWand->getData());

      mCameraPos = *(mCamera->getData());
      mCameraPos.invert(mCameraPos);         // Invert to get viewing version
   }

private:
   // Drawing attributes
   bool     mDrawProjections;    //: Should we draw projections
   vjVec3   mSurfaceColor;       //: Color to draw surfaces

   /// Defines the projection for this window. Ex. RIGHT, LEFT, FRONT
   vjProjection*   mCameraProj;            // Camera projection. (For sim, etc.)

   vjPosInterface mCamera;     // Prosy interfaces to devices needed
   vjPosInterface mWand;

   vjMatrix    mCameraPos;    // The data about the position of all this stuff
   vjMatrix    mHeadPos;
   vjMatrix    mWandPos;
};

#endif
