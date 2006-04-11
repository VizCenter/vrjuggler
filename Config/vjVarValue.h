/*
 *  File:	    $RCSfile$
 *  Date modified:  $Date$
 *  Version:	    $Revision$
 *
 *
 *                                VR Juggler
 *                                    by
 *                              Allen Bierbaum
 *                             Christopher Just
 *                             Patrick Hartling
 *                            Carolina Cruz-Neira
 *                               Albert Baker
 *
 *                         Copyright  - 1997,1998,1999
 *                Iowa State University Research Foundation, Inc.
 *                            All Rights Reserved
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
    static std::string using_invalid_msg;

public:

    //:Gets a reference to a global "invalid" vjVarValue
    //!NOTE: This is mainly useful for returning an invalid VarValue in
    //+      case of an error, and is used internally by some Config/*
    //+      classes.
    //!NOTE: There is a farily harmless race condition where an extra
    //+      invalid vjVarValue gets created & not deleted.  This is 
    //+      very unlikely, and would only result in losing a few bytes
    //+      anyway.
    static vjVarValue& getInvalidInstance ();


    inline VarType getType () {
	return type;
    }

    //: Copy constructor.
    vjVarValue (vjVarValue &v);


    //: Constructor - creates a T_EMBEDDEDCHUNK vjVarValue containing ch
    vjVarValue (vjConfigChunk* ch);


    //: Creates a new vjVarValue of type t.
    //! NOTE: Note that once a vjVarValue object has been created, the type
    //+ cannot be changed.
    vjVarValue ( VarType t );



    //: Destroys self and all associated memory.
    ~vjVarValue();



    //: Assignment Operator
    vjVarValue& operator= (const vjVarValue &v);



    //: Equality Operator
    bool operator == (const vjVarValue& v);
    inline bool operator != (const vjVarValue& v) {
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
    operator int();
    

    //: cast to ConfigChunk
    //!NOTE: Returns a copy of the contained chunk which must be
    //+      freed.
    operator vjConfigChunk*();


    //: Cast to bool
    operator bool();


    //: Cast to float (for T_FLOAT or T_DISTANCE)
    operator float ();


    //: Returns a string VarValue as a c-style string
    //!NOTE: returns a freshly allocated char array that the caller is 
    //+      responsible for deleting.
    char* cstring ();


    //: Cast to std::string
    operator std::string ();


    //: Assignment overload 
    //!NOTE: type of a vjVarValue is immutable, so a type mismatch here
    //+      can cause an error (in which case the assignment fails)
    vjVarValue& operator = (int i);
    vjVarValue& operator = (bool i);
    vjVarValue& operator = (float i);
    vjVarValue& operator = (std::string i);

    //: Assignment overload 
    //!NOTE: type of a vjVarValue is immutable, so a type mismatch here
    //+      can cause an error (in which case the assignment fails)
    //!NOTE: the vjVarValue makes a copy of the string - you can do with
    //+      the original as you please.
    vjVarValue &operator = (char *s);

    vjVarValue &operator = (vjConfigChunk *s);



    //: Writes the value of self to the stream out
    //!NOTE: v knows what type it is, so it makes sure it's printed
    //+      in a reasonable way.  ints & floats are printed as numbers,
    //+      bools as the strings "true" and "false", strings and 
    //+      chunks as their string reps, etc.
    friend ostream& operator << (ostream& out, const vjVarValue& v);
    
};

#endif
