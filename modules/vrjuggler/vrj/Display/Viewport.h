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

#ifndef _VRJ_VIEWPORT_H_
#define _VRJ_VIEWPORT_H_

#include <vrj/vrjConfig.h>

#include <gmtl/Vec.h>
#include <jccl/PerfMonitor/LabeledPerfDataBuffer.h>
#include <jccl/Config/ConfigChunkPtr.h>
#include <jccl/PerfMonitor/PerformanceMonitor.h>

namespace vrj
{

class Display;
class Projection;
class User;

/**
 * Base class for window viewports.  Base class for all viewport data.
 * Stores projection data, viewport info, and relevant user.
 *
 * @date 3-5-2001
 */
class VJ_CLASS_API Viewport
{
public:
   Viewport()
      : mUser(NULL), mDisplay(NULL),
        mLatencyMeasure(), mLeftProj(NULL), mRightProj(NULL)
   {
      mXorigin = mYorigin = mXsize = mYsize = -1.0f;
      mType = Viewport::UNDEFINED;
      mActive = false;
   }

   virtual ~Viewport()
   {;}

   // XXX: Hack, should be removed
   void recordLatency (int trackertimeindex, int currenttimeindex);

   /** Type of viewport */
   enum Type
   {
      UNDEFINED,  /**< Undefined type */
      SURFACE,    /**< Surface viewport */
      SIM         /**< Simulator viewport */
   };

   /** Used for referencing which eye(s) to draw */
   enum View
   {
      NONE=0,       /**< Neither eye */
      LEFT_EYE=1,   /**< Left eye only */
      RIGHT_EYE=2,  /**< Right eye only */
      STEREO=3      /**< Both left and right eyes */
   };

public:
   /**
    * Takes a viewport chunk and configures the display based one it.
    * @pre chunk is a valid chunk.
    * @post viewport is configured.
    *        If there is an error is the specified config, we output error
    *        and "fix" the error.
    *
    * @note All derived viewport classes MUST call this function
    *        after doing local configuration.
    */
   virtual void config(jccl::ConfigChunkPtr chunk);

   /**
    * Updates the projection data for this display.
    * Uses the data for the head position for this window.
    */
   virtual void updateProjections() = 0;

public:
   Viewport::Type getType()
   { return mType;}

   bool isSimulator()
   { return (mType == SIM); }

   bool isSurface()
   { return (mType == SURFACE); }

   bool isActive()
   { return mActive; }

    void setName(std::string name)
   { mName = name; }

   /** Gets the name of this display. */
   std::string getName()
   { return mName;}

   /**
    * @note If we are in simulator, we can not be in stereo.
    */
   bool inStereo()
   { return (mView == STEREO); }

   // Which view are we supposed to render
   Viewport::View getView()
   { return mView; }

   void setOriginAndSize(float xo, float yo, float xs, float ys)
   { mXorigin = xo; mYorigin = yo; mXsize = xs; mYsize = ys;}
   void getOriginAndSize(float& xo, float& yo, float& xs, float& ys)
   {
      xo = mXorigin; yo = mYorigin; xs = mXsize; ys = mYsize;
   }

   /** Gets the config chunk that configured this display. */
   jccl::ConfigChunkPtr getConfigChunk()
   { return mViewportChunk; }

   /** Gets the user associated with this display. */
   User*  getUser()
   { return mUser;}

   void setDisplay(Display* disp)
   { mDisplay = disp; }
   Display* getDisplay()
   { return mDisplay; }

   Projection* getLeftProj()
   { return mLeftProj; }

   Projection* getRightProj()
   { return mRightProj; }


   virtual std::ostream& outStream(std::ostream& out);
   friend std::ostream& operator<<(std::ostream& out, Viewport& viewport);

protected:
   std::string       mName;               /**< The name of the viewport being displayed */
   User*             mUser;               /**< The user being rendered by this window */
   Viewport::Type    mType;               /**< The type of display */
   Viewport::View    mView;               /**< Which buffer(s) to display (left, right, stereo) */
   bool              mActive;             /**< Is this viewport active */

   Display*          mDisplay;            /**< The parent display */

   jccl::ConfigChunkPtr mViewportChunk;        /**< The chunk data for this display */
   jccl::LabeledPerfDataBuffer mLatencyMeasure;

   /** @name Location and size of viewport
   * ASSERT: all values are >= 0.0 and <= 1.0
   */
   //@{
   float          mXorigin, mYorigin, mXsize, mYsize;
   //@}

   /** @name Projections
   * Defines the projection for this window
   */
   //@{
   Projection*   mLeftProj;      /** Left eye projection */
   Projection*   mRightProj;     /** Right eye projection */
   //@}

};

};

#endif
