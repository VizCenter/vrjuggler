
#ifndef _VJ_PROPERTY_H_
#define _VJ_PROPERTY_H_

#include <Config/vjVarValue.h>
#include <Config/vjChunkDesc.h>


//------------------------------------------------------------------
//: Stores a property and all its values
//
// Unit of storage inside a vjConfigChunk.  Has a name, type,
// and 0 or more values.  Some vjPropertys have a fixed number
// of values (e.g. to store the three components of a point
// in 3space), while others have a variable number (e.g. to
// store a list of active devices)
// <p>Each instance is associated with
// a vjPropertyDesc at instantiation; this associated cannot
// be changed.
// <p>Note that the Property instance maintains a pointer to
// the PropertyDesc, so be careful.
//
// <p>author: Christopher Just
//
//------------------------------------------------------------------
class vjProperty {

public:

    //: Pointer to this vjProperty's description.
    vjPropertyDesc *description;      

    //: Name of the vjProperty (shortcut for description->name)
    char *name;

    //: Number of values.  -1 for variable number (use getNum() )
    int num;

    //: Type of value entries.
    VarType type;                   

    //: A unit, if type is T_DISTANCE. (not fully functional)
    CfgUnit units;                  

    //: Vector containing the actual vjVarValues.
    vector<vjVarValue*> value;        



    //: Constructor
    //! PRE: true
    //! POST: Property is created.  If num values is not -1, num
    //+       vjVarValues are created and placed in value.
    //+       Otherwise, value is left empty.
    //! ARGS: pd - a pointer to a valid vjPropertyDesc. 
    //! NOTE: Self stores a pointer to its PropertyDesc pd.  pd
    //+       should not be deleted while self exists.
    vjProperty (vjPropertyDesc *pd);



    //: Destructor
    //! PRE: true
    //! POST: self and its stored values are destroyed (but not
    //+       the PropertyDesc!)
    ~vjProperty ();



    vjProperty (vjProperty& p);



    vjProperty& operator= (vjProperty& p);



    //: Returns actual current number of values in self
    //! RETURNS: n - size of value vector.
    int getNum ();



    //: Returns the VarValue at index
    //! PRE: True
    //! ARGS: ind - integer index of value to return (default 0)
    //! RETURNS: v - indth element of value, or a T_INVALID VarValue
    //+          if ind is out of bounds
    vjVarValue& getValue (int ind = 0);



    //: set the value at ind
    //! PRE: true
    //! POST: the indth value of self is set to val, if ind is
    //+       a valid index to self's value vector. If self has
    //+       a variable number of values and ind is greater than
    //+       the current number of values, the value vector will
    //+       be padded with VarValues of the appropriate type
    //+       (with default values).
    //! NOTE: If the argument can't be assigned because of type
    //+       mismatch, the value at ind won't be changed.
    //+       See vjVarValue::= to see what happens.  Padding
    //+       of the value vector may still occur.
    //! ARGS: val - value to assign.  If char*, must be a valid
    //+       non-NULL C string.
    //! ARGS: ind - integer index to value vector
    bool setValue (int val, int ind = 0);
    bool setValue (float val, int ind = 0);
    bool setValue (char* val, int ind = 0);



  vjEnumEntry* getEnumEntry (char *n);
  vjEnumEntry* getEnumEntry (int val);



  /** Converts the values in this property from units of u to units of feet.
   *  This is used when we're reading in a new distance property - The
   *  values have been read in as raw numbers, then we read in the unit type
   *  and realize what kind of conversion we have to do to put them in
   *  feet (our standard unit for internal storage of distances).
   */
  bool applyUnits (CfgUnit u);


    //: writes p to out
    friend ostream& operator << (ostream &out, vjProperty& p);



private:

  //: Utility function for setValue(val, ind)
  //! POST: If self has a variable number of values, and ind
  //+       is greater than the current size of the value
  //+       vector, the vector is padded with new default-valued
  //+       vjVarValues.
  //! RETURNS: true if ind is a valid index to the values vector
  //+          (after padding).
  //! RETURNS: false if ind is out of bounds.
  //! ARGS: ind - index into the values vector.
  bool preSet (int ind);

};


#endif
