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



#ifndef _VJ_PROPERTY_DESC_H_
#define _VJ_PROPERTY_DESC_H_

#include <vjConfig.h>
#include <Config/vjEnumEntry.h>


//------------------------------------------------------------
//: A Description used to instantiate a vjProperty
//
//       Information stored in a vjPropertyDesc includes vjProperty
//       Name, Type, number of allowed values, and a Help string
//       describing the purpose of the particular property.
//       vjPropertyDescs also include information for parsing a
//       vjProperty, and (optional) enumeration data for T_INT
//       type Properties.
//       Note: Frequently the docs for this class will refer to 'this
//       vjProperty', which refers to any object of class vjProperty
//       instantiated
//       using this description.
//
// @author:  Christopher Just
//------------------------------------------------------------

class vjPropertyDesc {

public:

    //: Constructor
    //!POST: name, token, help = NULL, type = T_INVALID, num = 0,
    //+      valuelabels & enumerations are empty.
    vjPropertyDesc ();



    //:Copy Constructor
    vjPropertyDesc (const vjPropertyDesc& d);



    //: Convenience constructor
    //!POST: name = token = n, help = h, num = i, type = t,
    //+      valuelabels & enumerations are empty.
    vjPropertyDesc (const std::string& n, int i, VarType t, const std::string& h);


    //: Destroys a vjPropertyDesc, and frees all allocated memory.
    ~vjPropertyDesc ();



    #ifdef VJ_DEBUG
    void assertValid () const;
    #else
    inline void assertValid () const {
        ;
    }
    #endif



    //: returns the token string for
    inline std::string& getToken () {
        return token;
    }


    inline std::string& getName () {
        return name;
    }



    inline VarType getType () {
        return type;
    }


    inline int getNumAllowed () {
        return num;
    }


    //: Returns the number of individual value labels
    inline int getValueLabelsSize () {
        return valuelabels.size();
    }


    //: Returns the ith value label
    std::string getValueLabel (unsigned int index);



    //: Returns the enumeration entry at index ind
    //! ARGS: index - index of EnumEntry to retrieve (0-base)
    //! RETURNS: NULL - if index is < 0 or out of range
    //! RETURNS: enumentry* - otherwise
    vjEnumEntry* getEnumEntryAtIndex (unsigned int index);


    //: Returns an enumentry with val matching val...
    vjEnumEntry* getEnumEntryWithValue (vjVarValue& val);


    //: Returns the enumentry named _name
    //! RETURNS: NULL - if no match if found
    //! RETURNS: vjEnumEntry* - otherwise
    vjEnumEntry* getEnumEntry (const std::string& _name);


    //: Writes a vjPropertyDesc to the given ostream
    //!NOTE: output format is:
    //+      name typename num token { enum1 enum2=42 } "help string"
    friend std::ostream& operator << (std::ostream& out, vjPropertyDesc& self);



    //: Reads a vjPropertyDesc from the named istream
    //!NOTE: format is the same as that written out by <<
    friend std::istream& operator >> (std::istream& in, vjPropertyDesc& self);


    vjPropertyDesc& operator= (const vjPropertyDesc& pd);

    //: Equality Operator
    // BUG (IPTHACK) - doesn't check equality of enumerations and valuelabels
    bool operator== (const vjPropertyDesc& pd);

    //: Inequality Operator
    inline bool operator!= (const vjPropertyDesc& pd) {
        return !(*this == pd);
    }

private:

    //: Descriptive name of the vjProperty this object describes. Used in GUI.
    std::string name;

    //: Short name for this vjPropertyDesc.  Used in app/library code.
    std::string token;

    //: One line of help information for this vjPropertyDesc.
    std::string help;

    //: Type of values allowed in this vjProperty.
    VarType type;

    //: Number of value entries allowed for this vjProperty. (-1 for variable)
    //  Typically this is an integer > 0.  For example, a tracker
    //  position offset might be described with 3 Float values (xyz).
    //  A value of -1 indicates that this vjProperty may have a variable
    //  number of values (e.g. for a list of active Walls).
    int  num;

    //: Labels for individual values of this Property (ie. "width", "height")
    std::vector<vjEnumEntry*> valuelabels;

    //: A list of labeled values that are allowed.
    //  string/int pairs for T_INTs,
    //  valid string values for T_STRINGS, and names of acceptable chunk
    //  types for T_CHUNK.  Note that in the T_CHUNK case, an empty enumv
    //  means _all_ chunk types are accepted
    std::vector<vjEnumEntry*> enumv;

    unsigned int validation;

};


#endif
