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



#ifndef _VJ_VARVALUE_H_
#define _VJ_VARVALUE_H_

#include <vjConfig.h>
#include <ctype.h>
#include <string.h>

typedef enum { T_INT, T_FLOAT, T_BOOL, T_STRING, T_DISTANCE, 
	       T_CHUNK, T_EMBEDDEDCHUNK, T_INVALID } VarType;

typedef enum {U_Feet, U_Inches, U_Meters, U_Centimeters, U_BadUnit}
              CfgUnit;

class vjConfigChunk;

/* note for myself as I'm adding T_DISTANCE - everything gets stored
 * internally as feet.
 */

//-------------------------------------------------
//: A vjVarValue is an object that knows its own type even if we don't.
//  More seriously, it's the value storage unit and value return type
//  for a ConfigChunk. <br>
//  Currently, vjVarValues can be of types int, FLOAT, boolean, string
//  (char*), distance(essentially FLOAT), as defined by the VarType
//  enumeration in vjVarValue.h. <br>
//  When you get a vjVarValue, you can do just a few things with it: <br>
//    1. assign it to a variable and then use it.  Note that there is
//       type checking here: if you try assigning a string vjVarValue to
//       an int, you'll get an error. <br>
//    2. Cast it to the right type and use it. <br>
//    3. print it - vjVarValues have overloaded << so you can print them
//       without having to cast to the right value. <br>
//  Note that it's generally incumbent upon the client to know what
//  kind of vjVarValue he's getting and what it can do.  Hey, you're
//  the one who queried the ConfigChunk, not me. <br>
//
// @author  Christopher Just
//
//!PUBLIC_API:
//--------------------------------------------------

class vjVarValue {

private:

    VarType    type;

    // these are the possible storage areas.
    int          intval;
    float        floatval;
    std::string  strval;
    bool         boolval;
    vjConfigChunk *embeddedchunkval;

    static vjVarValue* invalid_instance;
    static const std::string using_invalid_msg;

public:

    //:Gets a reference to a global "invalid" vjVarValue
    //!NOTE: This is mainly useful for returning an invalid VarValue in
    //+      case of an error, and is used internally by some Config/*
    //+      classes.
    //!NOTE: There is a fairly harmless race condition where an extra
    //+      invalid vjVarValue gets created & not deleted.  This is 
    //+      very unlikely, and would only result in losing a few bytes
    //+      anyway.
    static vjVarValue& getInvalidInstance ();


    inline VarType getType () const {
	return type;
    }

    //: Copy constructor.
    vjVarValue (const vjVarValue &v);


    //: Constructor - creates a T_EMBEDDEDCHUNK vjVarValue containing ch
    //!NOTE: This is explicit for safety's sake.  I already encountered a bug
    //+      where some chunkdb code was interpreting
    //+          vjVarValue v1 = chunk.getProperty(blah)
    //+      as casting the result of getProperty to a chunk* and then calling
    //+      this constructor instead of using the vjVarValue copy constructor
    //+      becuase getProperty returns a const vjVarValue and the copy const
    //+      didn't expect a const (since fixed).
    explicit vjVarValue (vjConfigChunk* ch);


    //: Creates a new vjVarValue of type t.
    //! NOTE: Note that once a vjVarValue object has been created, the type
    //+ cannot be changed.
    vjVarValue ( VarType t );



    //: Destroys self and all associated memory.
    ~vjVarValue();



    //: Assignment Operator
    vjVarValue& operator= (const vjVarValue &v);



    //: Equality Operator
    bool operator == (const vjVarValue& v) const;
    inline bool operator != (const vjVarValue& v) const {
	return !(*this == v);
    }


    /*  Cast Operators
     *  These operators are used whenever a vjVarValue is cast to another
     *  type.  They do some amount of type checking and coercion,
     *  eventually returning the data stored within the config itself.
     *  Right now, in event of an error we only write a message to cerr
     *  and return a "reasonable" value - 0, 0.0, false, "", NULL, etc.
     */

    //: Cast to int
    //!RETURNS: i - integer value of self if T_INT, 0 or 1 if T_BOOL
    //!RETURNS: 0 - if not T_INT or T_BOOL (this is bad)
    operator int() const;
    

    //: cast to ConfigChunk
    //!NOTE: Returns a copy of the contained chunk which must be
    //+      freed.
    operator vjConfigChunk*() const;


    //: Cast to bool
    operator bool() const;


    //: Cast to float (for T_FLOAT or T_DISTANCE)
    operator float () const;


    //: Returns a string VarValue as a c-style string
    //!NOTE: returns a freshly allocated char array that the caller is 
    //+      responsible for deleting.
    char* cstring () const;


    //: Cast to std::string
    operator std::string () const;


    //: Assignment overload 
    //!NOTE: type of a vjVarValue is immutable, so a type mismatch here
    //+      can cause an error (in which case the assignment fails)
    vjVarValue& operator = (int i);
    vjVarValue& operator = (bool i);
    vjVarValue& operator = (float i);
    vjVarValue& operator = (const std::string& i);

    //: Assignment overload 
    //!NOTE: type of a vjVarValue is immutable, so a type mismatch here
    //+      can cause an error (in which case the assignment fails)
    //!NOTE: the vjVarValue makes a copy of the string - you can do with
    //+      the original as you please.
    vjVarValue &operator = (const char *s);

    vjVarValue &operator = (vjConfigChunk *s);



    //: Writes the value of self to the stream out
    //!NOTE: v knows what type it is, so it makes sure it's printed
    //+      in a reasonable way.  ints & floats are printed as numbers,
    //+      bools as the strings "true" and "false", strings and 
    //+      chunks as their string reps, etc.
    friend ostream& operator << (ostream& out, const vjVarValue& v);
    
};

#endif
