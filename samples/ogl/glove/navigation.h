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

#ifndef NAVIGATION_H
#define NAVIGATION_H

class TrackedInfo
{
public:
    //: call this once per frame with your tracker's matrix.
    void updateWithMatrix( vjMatrix& matrix );
    
    const vjVec3& vector() const { return _vec; }
    const vjVec3& rotDelta() const { return _rotDelta; }
    const vjVec3& rotation() const { return _rot; }
    
    
private:
    vjVec3 _vec, _rot, _rotOld;
    vjVec3 _rotDelta;
    
    //: a vector pointing forward in our space, 
    //  useful for getting what direction a device is pointing.
    static const vjVec3 forwardVec;
    
    //: the origin
    static const vjVec3 origin;
};

class UserInfo
{
public:
    //: default constructor
    UserInfo();
    
    //: set the "velocity per frame" once each frame.
    //  required - call this before you use any 'update' functions.
    void  setVelocity( const float& velocity );
    
    //: set the "angular velocity per frame" once each frame.
    //  required - call this before you use any 'update' functions.
    //  give - aVelocity, a value from [0,1]
    void  setAngularVelocity( const float& aVelocity );

    //: call this once per frame with the tracker's TrackerInfo
    //  this will update user data such as position, velocity
    //  NOTE: if in "weightless" mode, 
    //        then pass (0,0,0) in for gravity
    void  update( const TrackedInfo& tracker, const vjVec3& gravity );
    
    //: get the transform to put the scene from the user's point of view
    //  from the user's info, calculate, then return, the  
    //  transform to put the scene into the user's point of view
    void  getSceneTransform( vjMatrix& sceneMatrtix ) const;
    
    inline void move( vjVec3& dist )
    { move( dist[0], dist[1], dist[2] ); }
    
    inline void move( float a, float b, float c )
    { _pos[0] += a; 
      _pos[1] += b;
      _pos[2] += c;
    }
    
    inline void reset()
    {
	_pos.set(0, 0, 0);
	_posOld.set(0, 0, 0);
	_rot.set(0, 0, 0);
	_rotOld.set(0, 0, 0);
	_velocity = 0.0f;
	_velocityVec.set(0, 0, 0);
	_angularVelocity = 0;
    }
    
    
    inline bool&  walk()
    {
	return _walkingMode;
    }
    
private:

    void _updateWithTracker( const TrackedInfo& tracker );
    void _updateWithGravity( const vjVec3& gravity );

    // current and old position
    vjVec3  _pos, _posOld;
    
    // current and old rotations
    vjVec3  _rot, _rotOld;
    
    // velocity vector
    vjVec3  _velocityVec;
    
    // velocity per frame
    float _velocity;
    
    // angular velocity
    float _angularVelocity;
    
    // are we in walking or weightless mode?
    bool _walkingMode;
};

#endif
