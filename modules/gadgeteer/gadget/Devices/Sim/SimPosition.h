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

#ifndef _GADGET_SIM_POSITION_H
#define _GADGET_SIM_POSITION_H
//#pragma once

#include <gadget/gadgetConfig.h>
#include <gadget/Type/Input.h>
#include <gadget/Devices/Sim/SimInput.h>
#include <gadget/Type/Position.h>
#include <gadget/Type/InputMixer.h>
#include <gmtl/Vec.h>
#include <gmtl/VecOps.h>

namespace gadget
{

/**
 * Simulated positional device.
 * Simulates a positional device from a keyboard device.
 * This class should not be used directly by the user.
 */
//class SimPosition :  public SimInput, public Input, public Position
class SimPosition : public InputMixer<InputMixer<SimInput,Input>,Position>
{
public:
   /** Constants for the key array. */
   enum {
      FORWARD = 0,
      BACK    = 1,
      LEFT    = 2,
      RIGHT   = 3,
      UP      = 4,
      DOWN    = 5,
      ROTR    = 6,
      ROTL    = 7,
      ROTU    = 8,
      ROTD    = 9,
      ROT_ROLL_CCW = 10,
      ROT_ROLL_CW = 11,
      NUM_POS_CONTROLS = 12
   };

   /** Const for coord system selection. */
   enum {
      LOCAL = 0,
      GLOBAL = 1
   };
public:
   SimPosition()
      : mTransCoordSystem(LOCAL), mRotCoordSystem(LOCAL),
        mDTrans(-1221.75f), mDRot(-1221.75f)
   {;}
   virtual ~SimPosition() {;}

   virtual bool config(jccl::ConfigChunkPtr chunk);

   /** These functions don't do anything. */
   int startSampling() { return 1; }
   int stopSampling() { return 1; }
   int sample() { return 1; }

   /** Updates the data. */
   virtual void updateData();

   static std::string getChunkType() { return std::string("SimPosition"); }

private:
   /** @name Movement helpers */
   //@{
   /** Move amt in direction.
   * Captures code common for all movements.
   */
   void moveDir(const float amt, const gmtl::Vec3f dir);

   /** Moves forward (-z) the given amount on position data n. */
   void moveFor(const float amt);

   /** Moves left (-X) the given amount. */
   void moveLeft(const float amt);

   /** Moves up (+y) the given amount. */
   void moveUp(const float amt);

   /** @name Rotation helpers */
   //@{
   /** Rotation amt around axis.
   * Captures code common for all rotations.
   */
   void rotAxis(const float amt, const gmtl::Vec3f rotAxis);

   /** Pitch up - rot +x axis. */
   void rotUp(const float amt);

   /** Yaw left - rot +Y axis. */
   void rotLeft(const float amt);

   /** Roll Left - rot -z axis. */
   void rotRollCCW(const float amt);
   //@}

   /**
    * Checks if movement is allowed.
    * @note It is not allowed if it hits a simulated wall, etc.
    */
   bool isTransAllowed(gmtl::Vec3f trans);


private:
   int            mTransCoordSystem;            /**< What coord system for translations? */
   int            mRotCoordSystem;              /**< What coord system for rotations? */
   PositionData   mPos;                         /**< The current position being simulated */
   KeyModPair     mSimKeys[NUM_POS_CONTROLS];   /**< The keys to press for digital simulation */
   float          mDTrans;                      /**< Delta translation */
   float          mDRot;                        /**< Delta rotation */

};

} // End of gadget namespace

#endif
