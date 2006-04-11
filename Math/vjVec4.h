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


#ifndef _VJ_VEC4_
#define _VJ_VEC4_
//#pragma once

#include <vjConfig.h>
#include <math.h>

#include <Math/vjMatrix.h>
class vjMatrix;

// --- Define access reps --- //
#define VJ_X 0
#define VJ_Y 1
#define VJ_Z 2
#define VJ_W 3

//: Vector of length 4
//!PUBLIC_API:
class vjVec4
{
public:
   // Constructor
   vjVec4(float _x, float _y, float _z, float _w)
   { set(_x, _y, _z, _w);}
   vjVec4() { set(0.0f,0.0f,0.0f,0.0f);}

public:
   //: Set value of Vector
   void set(float _x, float _y, float _z, float _w) {
      vec[0] = _x;
      vec[1] = _y;
      vec[2] = _z;
      vec[3] = _w;
   }

public:
   //: Are we equal
   bool equal(const vjVec4&  _v) const {
      return (vec[0] == _v[0] &&
              vec[1] == _v[1] &&
              vec[2] == _v[2] &&
              vec[3] == _v[3]);
   }

   float dot(const vjVec4&  _v) const {
      return (vec[0] * _v[0] +
              vec[1] * _v[1] +
              vec[2] * _v[2]);
   }


   void normalize()
   {
      float len = length();
      vec[0] = vec[0] / len;
      vec[1] = vec[1] / len;
      vec[2] = vec[2] / len;
      vec[3] = vec[3] / len;
   }

   float length() const
   {
      return sqrtf((vec[0]*vec[0])+
                   (vec[1]*vec[1])+
                   (vec[2]*vec[2])+
                   (vec[3]*vec[3]));
   }


   //: Xform the vector
   // Set vec = (Mat)(Vec)
   void xform(const vjMatrix& _m, vjVec4 _v);

public:
   /// Operators
   float&  operator [](int i) { return vec[i];}
   const float&  operator [](int i) const { return vec[i];}

   int operator ==(const vjVec4& _v) const {
      return (vec[0] == _v[0] &&
              vec[1] == _v[1] &&
              vec[2] == _v[2] &&
              vec[3] == _v[3]);
   }

   int operator !=(const vjVec4& _v) const {
      return !(*this == _v);
   }

public:
   // vjVec4 operators, return by value could be slow
   vjVec4 operator -() const {
      return vjVec4(-vec[0], -vec[1], -vec[2], -vec[3]);
   }

   vjVec4 operator +(const vjVec4& _v) const {
      return vjVec4(vec[0]+_v[0], vec[1]+_v[1], vec[2]+_v[2], vec[3]+_v[3]);
   }

   vjVec4 operator -(const vjVec4& _v) const {
      return vjVec4(vec[0]-_v[0], vec[1]-_v[1], vec[2]-_v[2], vec[3]-_v[3]);
   }

   friend inline vjVec4 operator *(float _s, const vjVec4&);
   friend inline vjVec4 operator *(const vjVec4& _v, float _s);
   friend inline vjVec4 operator /(const vjVec4& _v, float _s);

public:
   //: Assignment Operators
   vjVec4&  operator =(const vjVec4& _v) {
      vec[0] = _v[0]; vec[1] = _v[1];
      vec[2] = _v[2]; vec[3] = _v[3];
      return *this;
   }

   vjVec4& operator *=(float _s) {
      vec[0] *= _s; vec[1] *= _s;
      vec[2] *= _s; vec[3] *= _s;
      return *this;
   }

   vjVec4& operator /=(float _s) {
      _s = 1.0/_s;
      return *this *= _s;
   }

   vjVec4& operator +=(const vjVec4& _v) {
      vec[0] += _v[0]; vec[1] += _v[1];
      vec[2] += _v[2]; vec[3] += _v[3];
      return *this;
   }

   vjVec4& operator -=(const vjVec4& _v) {
      vec[0] -= _v[0]; vec[1] -= _v[1];
      vec[2] -= _v[2]; vec[3] -= _v[3];
      return *this;
   }

public:
   float vec[4];        //: The vector data
};

inline vjVec4 operator *(float _s, const vjVec4& _v);
inline vjVec4 operator *(const vjVec4& _v, float _s);
inline vjVec4 operator /(const vjVec4& _v, float _s);
std::ostream& operator<<(std::ostream& out, vjVec4& _v);

#endif
