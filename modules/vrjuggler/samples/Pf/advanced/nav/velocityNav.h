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
#ifndef _VELOCITY_NAV_H
#define _VELOCITY_NAV_H

#include <gmtl/Quat.h>
#include <gmtl/QuatOps.h>
#include <navigator.h>
#include <collider.h>
#include <vector>
#include <vpr/Util/Interval.h>
//#include "StopWatch.h"

class velocityNav : public navigator
{
public:
   enum navMode { DRIVE=0, FLY=1 };
   velocityNav();

   // Empty virtual destructor to make GCC happy.
   virtual ~velocityNav (void) {
      /* Do nothing. */ ;
   }

   // Update the interaction state
   virtual void updateInteraction();

   // Update the navigation position
   virtual void update();

   // Set the action buttons that can be used
   void setActionButtons(std::vector<std::string> action_btn_names);

   // Set the name of the pos device that is used in navigation
   void setNavPosControl(std::string wand_dev);

   // Specify the combination of keys necessary to trigger the given state
   void setAccelForwardActionCombo(std::vector<ActionState> combo) { mAccelForwardCombo = combo; }
   void setBrakeActionCombo(std::vector<ActionState> combo) { mBrakeCombo = combo; }
   void setStopActionCombo(std::vector<ActionState> combo) { mStopCombo = combo; }
   void setRotationActionCombo(std::vector<ActionState> combo) { mRotateCombo = combo; }
   void setResetActionCombo(std::vector<ActionState> combo) { mResetCombo = combo; }

   // pass in velocity to add per second
   void setAccel( const float& vel_per_sec ) { mAcceleration = vel_per_sec; }

   // set the highest velocity this nav can achieve.
   // default is 35
   void setMaxVelocity( const float& velocity = 50.0f )
   { mMaxVelocity = velocity; }

   //: arbitrary acceleration in any direction.
   //  nice for straefing, rising or diving. :)
   //  accel = velocity per second
   void accelerate(const gmtl::Vec3f& accel);

   //: zero all velocity
   void stop();

   // TODO: add other directions to accelerate. (since it's hard coded to 0,0,-x in updateInteraction)
   bool acceleratingForward() { return mAcceleratingForward;}
   bool braking() { return mBraking; }
   bool rotating() { return mRotating;}
   bool stopping() { return mStopping;}
   bool resetting() { return mResetting;}

   //: damping [0...1], where 0 stops immediately, .99 brakes very slowly
   //  default is no damping (1.0f)
   void setDamping( const float& damping = 1.0f );

   void setMode(navMode new_mode)
   { mMode = new_mode; }

   virtual gmtl::Vec3f getVelocity() const { return mVelocity; }
   virtual float  getSpeed() const { return gmtl::length(mVelocity); }

   // resets the navigator's matrix to the origin (set by setOrigin)
   virtual void reset();

protected:
   // set the rotational acceleration matrix
   // only rotational components of this matrix are used.
   void setRotationalAcceleration( const gmtl::Matrix44f& mat )
   { mRotationalAcceleration = mat;}

   // Rotate the environment
   void scaled_rotate(gmtl::Matrix44f rot_mat);

   // util func... used to convert gravity from meters...
   inline void meters2feet( const gmtl::Vec3f& gravityInMeters, gmtl::Vec3f& gravityInFeet )
   {
      float meters_to_feet = (100.0f / 0.3048f) * 0.01f;
      //float convtofeet = (meters / .3048 * 100.0f) * .01;
      gravityInFeet = gravityInMeters * meters_to_feet;
   }

   enum Units
   {
      METERS, FEET
   };

   void setUnits( Units units = FEET ) { mUnits = units; }

private:
   gmtl::Vec3f mVelocity;
   gmtl::Vec3f mVelocityFromGravityAccumulator;
   float    mMaxVelocity;
   gmtl::Matrix44f mRotationalAcceleration;
   float    mDamping;
   float    mAcceleration;

   // digitalInterfaces
   std::vector<gadget::DigitalInterface*> mActionButtons;
   gadget::PositionInterface mNavWand;

   // State combos
   std::vector<ActionState> mAccelForwardCombo;
   std::vector<ActionState> mBrakeCombo;
   std::vector<ActionState> mStopCombo;
   std::vector<ActionState> mRotateCombo;
   std::vector<ActionState> mResetCombo;

   // State flags for the navigation
   bool  mBraking;
   bool  mAcceleratingForward;
   bool  mRotating;
   bool  mStopping;
   bool  mResetting;

   float mTimeDelta;
   vpr::Interval mLastTimeStamp;

   Units       mUnits;
   //StopWatch   stopWatch;
   navMode     mMode;
   int         mTimeHack;
};

inline velocityNav::velocityNav() :
   mVelocity( 0.0f, 0.0f , 0.0f ),
   mVelocityFromGravityAccumulator(0.0f,0.0f,0.0f),
   mMaxVelocity( 2500.0f ),
   mDamping( 1.0f ),
   mAcceleration(10.0f),
   mUnits( velocityNav::FEET ),
   mMode( velocityNav::DRIVE ),
   mTimeHack(0),
   mLastTimeStamp(0,vpr::Interval::Base)
{
   stop();
   //stopWatch.start();
   //stopWatch.stop();

   setNavPosControl("VJWand");         // Initialize wand device

   std::vector<std::string> def_btns(3);
   def_btns[0] = std::string("VJButton0");
   def_btns[1] = std::string("VJButton1");
   def_btns[2] = std::string("VJButton2");
   setActionButtons(def_btns);

   std::vector<navigator::ActionState> accel_combo(3), brake_combo(3), stop_combo(3), rotate_combo(3), reset_combo(3);
   accel_combo[0] = ON;    accel_combo[1] = OFF;   accel_combo[2] = OFF;    // 100
   brake_combo[0] = OFF;   brake_combo[1] = ON;    brake_combo[2] = OFF;    // 010
   stop_combo[0] = ON;     stop_combo[1] = ON;     stop_combo[2] = OFF;     // 110
   rotate_combo[0] = OFF;  rotate_combo[1] = OFF;  rotate_combo[2] = ON;    // 001
   reset_combo[0] = OFF;   reset_combo[1] = ON;    reset_combo[2] = ON;     // 011

   setAccelForwardActionCombo(accel_combo);
   setBrakeActionCombo(brake_combo);
   setStopActionCombo(stop_combo);
   setRotationActionCombo(rotate_combo);
   setResetActionCombo(reset_combo);

   mBraking = mAcceleratingForward = mRotating = mStopping = mResetting = false;
}


// Set the action buttons that can be used
inline void velocityNav::setActionButtons(std::vector<std::string> action_btn_names)
{
   // Make sure list is long enough
   while(action_btn_names.size() > mActionButtons.size())
   {
      mActionButtons.push_back(new gadget::DigitalInterface());
   }

   // Update all the entries
   for(unsigned int i=0;i<action_btn_names.size();i++)
   {
      mActionButtons[i]->init(action_btn_names[i]);
   }
}

// Set the name of the pos device that is used in navigation
inline void velocityNav::setNavPosControl(std::string wand_dev)
{
   vprDEBUG(vprDBG_ALL,0) << clrOutNORM(clrGREEN,"Setting Nav Pos Control: ")
                        << wand_dev.c_str() << std::endl << vprDEBUG_FLUSH;
   mNavWand.init(wand_dev);
}

inline void velocityNav::updateInteraction()
{
   // If we are not supposed to be active, then don't run
   if(!this->isActive())
   {
      return;
   }

   mAcceleratingForward = checkForAction( mActionButtons, mAccelForwardCombo );
   mBraking = checkForAction( mActionButtons, mBrakeCombo );
   mStopping = checkForAction( mActionButtons, mStopCombo );
   mResetting = checkForAction( mActionButtons, mResetCombo );
   mRotating = checkForAction( mActionButtons, mRotateCombo );

   // Braking
   if(mBraking)
   { mDamping = 0.85; }
   else
   { mDamping = 1.0f; }

   // Stopping -- NOTE: This must go after braking
   if(mStopping)
   {
      setDamping(0.0f);
   }

   // Accelerate Forward
   // TODO: add other directions to accelerate. (since it's hard coded to forward (0,0,-x) here...)
   if(mAcceleratingForward)
   { accelerate(gmtl::Vec3f(0.0f, 0.0f, -mAcceleration)); }

   // Resetting
   if(mResetting)
      this->reset();

   // Rotating
   gmtl::Matrix44f rot_mat = *(mNavWand->getData());
   //std::cout<<"1: "<<rot_mat<<"\n"<<std::flush;
   gmtl::setTrans(rot_mat, gmtl::Vec3f(0.0f, 0.0f, 0.0f));
   //std::cout<<"2: "<<rot_mat<<"\n=======\n\n"<<std::flush;

   setRotationalAcceleration( rot_mat );

   // Output visual feedback
   if(mAcceleratingForward)
      vprDEBUG(vprDBG_ALL,0) << clrOutNORM(clrCYAN,"velNav: Accelerating Forward") << "(Accel = " << mAcceleration << ")" << std::endl << vprDEBUG_FLUSH;
   if(mBraking)
      vprDEBUG(vprDBG_ALL,0) << clrOutNORM(clrCYAN,"velNav: Braking")
                           << std::endl << vprDEBUG_FLUSH;
   if(mStopping)
      vprDEBUG(vprDBG_ALL,0) << clrOutNORM(clrCYAN,"velNav: Stopping")
                           << std::endl << vprDEBUG_FLUSH;
   if(mResetting)
   {
      gmtl::Vec3f hpos;
      gmtl::setTrans( hpos, mHomePos );
      vprDEBUG(vprDBG_ALL,0) << clrOutNORM(clrCYAN,"velNav: Resetting") << " to "
                           << hpos << std::endl << vprDEBUG_FLUSH;
   }
   if(mRotating)
       vprDEBUG(vprDBG_ALL,0) << clrOutNORM(clrCYAN,"velNav: Rotating")
                            << std::endl << vprDEBUG_FLUSH;
}

inline void velocityNav::update()
{
   //stopWatch.stop();
   //stopWatch.start();
   
   vpr::Interval cur_time = mNavWand->getTimeStamp();
   vpr::Interval diff_time(cur_time-mLastTimeStamp);
      
   mTimeDelta = diff_time.secf();
/*
   std::cout << "READANDWRITE Delta: " << diff_time.getBaseVal() << std::endl;
   std::cout << "READANDWRITE Current: " << cur_time.getBaseVal() << "Last: " << mLastTimeStamp.getBaseVal() << "\n" << std::endl;
*/      
   mLastTimeStamp = cur_time;



   if(mTimeDelta > 2.0f)    // If the time is greater than 2 seconds ( 1/2 fps)
   {
      vprDEBUG(vprDBG_ALL,0)
         << clrOutNORM(clrCYAN,"VelNav: timeInstant to large: ")
         << mTimeDelta << std::endl << vprDEBUG_FLUSH;
      //stopWatch.stop();    // Get a REALLY small delta time
      //stopWatch.start();
   }

   //vprDEBUG_BEGIN(vprDBG_ALL,0) << "VelNav: ----- Update ----\n" << vprDEBUG_FLUSH;
   //vprDEBUG(vprDBG_ALL,0) << "VelNav: timeInstant: " << mTimeDelta << std::endl << vprDEBUG_FLUSH;

   // If we are not supposed to be active, then don't run
   if(!this->isActive())
   {
      return;
   }

   //////////////////////////////////
   // do navigations...
   //////////////////////////////////
   // Define axes, in Juggler/OpenGL coordinate system (right handed)
   gmtl::Vec3f       trackerZaxis(0.0f, 0.0f, 1.0f);
   gmtl::Vec3f       trackerXaxis(1.0f, 0.0f, 0.0f);
   gmtl::Vec3f       trackerYaxis(0.0f, 1.0f, 0.0f);
   const gmtl::Vec3f gravity_meters_per_second( 0.0f, -9.8f, 0.0f ); //9.8 m/s (METERS)
   gmtl::Vec3f       gravity( 0.0f, -9.8f, 0.0f ); // to be set by switch (mUnits == METERS)

   switch (mUnits)
   {
   case FEET:
      meters2feet( gravity_meters_per_second, gravity );
      break;
   default:
   case METERS:
      gravity = gravity_meters_per_second;
      break;
   }

   if ((mAllowRot) && (mRotating))
   {
      this->scaled_rotate( mRotationalAcceleration );     // Interpolates the rotation, and updates mCurPos matrix
   }

   if (mMode == DRIVE)
   {
      // get the axes of the tracking/pointing device
      // NOTE: constrain to the Y axis in GROUND mode (no flying/hopping or diving faster than gravity allows)
      gmtl::Matrix44f constrainedToY;
      //mRotationalAcceleration.constrainRotAxis( false, true, false, constrainedToY );
      gmtl::EulerAngleXYZf euler( 0.0f,
                   gmtl::makeYRot(mRotationalAcceleration), 0.0f );
      gmtl::setRot( constrainedToY, euler ); // Only allow Yaw (rot y)

      gmtl::xform( trackerZaxis, constrainedToY, trackerZaxis );
      gmtl::xform( trackerXaxis, constrainedToY, trackerXaxis );
      gmtl::xform( trackerYaxis, constrainedToY, trackerYaxis );
   }
   else if (mMode == FLY)
   {
      // get the axes of the tracking/pointing device
      gmtl::xform( trackerZaxis, mRotationalAcceleration, trackerZaxis );
      gmtl::xform( trackerXaxis, mRotationalAcceleration, trackerXaxis );
      gmtl::xform( trackerYaxis, mRotationalAcceleration, trackerYaxis );
   }

   // this is used to accumulate velocity added by navigation
   gmtl::Vec3f velocityAccumulator( 0.0f, 0.0f, 0.0f );

   if (mMode == DRIVE)           // if DRIVING --> we have GRAVITY
   {
      // add the velocity this timeslice/frame by the acceleration from gravity.
      velocityAccumulator += mVelocityFromGravityAccumulator;

      //vprDEBUG(vprDBG_ALL,0) << "velNav: drive: gravAccum: " << mVelocityFromGravityAccumulator << vprDEBUG_FLUSH;

      // recalculate the current downward velocity from gravity.
      // this vector then is accumulated with the rest of the velocity vectors each frame.
      
      //mVelocityFromGravityAccumulator += (gravity * mTimeDelta);
      mVelocityFromGravityAccumulator += (gravity * mTimeDelta);

      //vprDEBUG_CONT(vprDBG_ALL,0) << " new vel: " << velocityAccumulator
      //                          << " new grav: " << mVelocityFromGravityAccumulator << endl << vprDEBUG_FLUSH;
   }
   if (mAllowTrans)
   {
      // add velocity with respect to the tracking/pointing device
      velocityAccumulator += (trackerZaxis * mVelocity[2]); // forward/reverse   |reletive to tracker
      velocityAccumulator += (trackerXaxis * mVelocity[0]); // strafe            |reletive to tracker
      velocityAccumulator += (trackerYaxis * mVelocity[1]); // rise/dive         |reletive to tracker
   }

   // Get rid of some velocity due to damping in the system
   mVelocity -= (mVelocity * (1.0f - mDamping));

   // navigation just calculated navigator's next velocity
   // now convert accumulated velocity to distance traveled this frame (by cancelling out time)
   // NOTE: this is not the final distance, since we still have to do collision correction.
   gmtl::Vec3f distanceToMove = velocityAccumulator * mTimeDelta;

   //vprDEBUG(vprDBG_ALL,0) << "velNav: distToMove = velAcum * instant: " << velocityAccumulator << " * " << mTimeDelta << endl << vprDEBUG_FLUSH;

   // --- TRANSLATION and COLLISION DETECTION --- //
   bool     did_collide;               // Did we collide with anything
   gmtl::Vec3f total_correction;          // The total correction on the movement (in modelspace)
   navTranslate(distanceToMove,did_collide,total_correction);

   if(did_collide)      // If we hit something, stop falling
   {
      //vprDEBUG(vprDBG_ALL,0) << "Did collide: Setting gravAccum to 0,0,0\n" << vprDEBUG_FLUSH;
      mVelocityFromGravityAccumulator.set( 0.0f, 0.0f, 0.0f );
   }
   //vprDEBUG_END(vprDBG_ALL,0) << "---------------------\n" << vprDEBUG_FLUSH;
}

inline void velocityNav::scaled_rotate(gmtl::Matrix44f rot_mat)
{
   //: Confused by quaternions???
   //  All this does is scale the angle of rotation back by about %4
   //  Quaternions allow you to interpolate between rotations,
   //  thus getting a scale factor of the original (with respect to identity)

   // I should scale the rotation matrix here
   // I should also compute a relative rotation from the head here as well
   gmtl::Matrix44f transform, transformIdent;
   gmtl::Quatf     source_rot, goal_rot, slerp_rot;

   // Create an identity quaternion to rotate from
   gmtl::identity(transformIdent);

   // Create the goal rotation quaternion (the goal is the input matrix)
   gmtl::set(goal_rot, rot_mat);

   // If we don't have two identity matrices, then interpolate between them
   if(transformIdent != rot_mat)
   {
      gmtl::slerp(slerp_rot, 0.04f, source_rot, goal_rot ); // Transform part way there
      gmtl::set( transform, slerp_rot ); // Create the transform matrix to use
   }
   else
   {
      gmtl::identity(transform);
   }
   navigator::navRotate(transform);                   // update the mCurPos navigation matrix
}

// accel = velocity per second
// TODO: hook this up to the vel accumulator...,
//        and get rid of mVelocity weirdness.
inline void velocityNav::accelerate(const gmtl::Vec3f& accel)
{
   if(gmtl::length(mVelocity) < mMaxVelocity)
   {
      mVelocity += (accel * mTimeDelta);
   }
}


inline void velocityNav::stop()
{
   this->setDamping( 0.0f );
}

inline void velocityNav::setDamping( const float& damping )
{
   mDamping = damping;
   //cout<<"Setting damping coef to: "<<mDamping<<"\n"<<flush;
}

inline void velocityNav::reset()
{
   mVelocity.set( 0,0,0 );
   mVelocityFromGravityAccumulator.set( 0,0,0 );
   gmtl::identity(mRotationalAcceleration);
   navigator::reset();
   //stopWatch.start();   // Reset the stop watch
   //stopWatch.stop();
}


#endif
