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



#ifndef _PARSEUTIL_H_
#define _PARSEUTIL_H_

#include <vjConfig.h>
#include <Config/vjVarValue.h>

bool readString (istream& in, char* buffer, int size, bool *quoted = NULL);
VarType readType (istream &ip);
char *typeString (VarType t);
char *unitString (CfgUnit t);
float toFeet (float val, CfgUnit unit);

//: same as stdlib strcasecmp, but args are std::strings
bool vjstrcasecmp (const std::string& a, const std::string& b);

//: strncasecmp(), but with strings as arguments
//! NOTE: if n < 0, or n is ommitted, n is set to the size of the shorter arg
//! NOTE: comparisons are case-insensitive
//! RETURNS: true - if the first n chars of a & b do NOT match
//! RETURNS: false - if the first n chars of strings a & b match
bool vjstrncasecmp (const std::string& a, const std::string& b, int _n = -1);

//: same as strncmp, but with std::strings as arguments
//! NOTE: if n < 0, or n is ommitted, n is set to the size of the shorter arg
//! RETURNS: true - if the first n chars of a & b do NOT match
//! RETURNS: false - if the first n chars of strings a & b match
bool vjstrncmp (const std::string& a, const std::string& b, int _n = -1);

//: mangles a filename n in several ways: replacing env variables & appending
//+ the parentfile's directory if n is a relative pathname
std::string demangleFileName (const std::string& n, std::string parentfile);

#endif
