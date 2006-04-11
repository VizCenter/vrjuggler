#!/bin/sh

# ************** <auto-copyright.pl BEGIN do not edit this line> **************
#
# VR Juggler is (C) Copyright 1998, 1999, 2000 by Iowa State University
#
# Original Authors:
#   Allen Bierbaum, Christopher Just,
#   Patrick Hartling, Kevin Meinert,
#   Carolina Cruz-Neira, Albert Baker
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.
#
# -----------------------------------------------------------------
# File:          $RCSfile$
# Date modified: $Date$
# Version:       $Revision$
# -----------------------------------------------------------------
#
# *************** <auto-copyright.pl END do not edit this line> ***************

branch='-1'
date='-1'
ver_file='-1'
input='-1'
output='-1'
subsystem='-1'
version='-1'

while [ $# -gt 0 ]; do
    case $1 in
        -b)
            branch="$2"
            shift
            shift
            ;;
        -d)
            date="$2"
            shift
            shift
            ;;
        -f)
            ver_file="$2"
            shift
            shift
            ;;
        -i)
            input="$2"
            shift
            shift
            ;;
        -o)
            output="$2"
            shift
            shift
            ;;
        -s)
            subsystem="$2"
            shift
            shift
            ;;
        -v)
            version="$2"
            shift
            shift
            ;;
    esac
done

if [ "x$date" = "x-1" -o "x$subsystem" = "x-1" -o "x$input" = "x-1" -o	\
     "x$output" = "x-1" ]
then
    echo "Usage: $0 [-b branch] -d <date> -i <input file> -o <output file>"
    echo "       -s <subsystem> [ -f <VERSION file> | -v <version> ]"
    exit 1
fi

if [ "x$ver_file" = "x-1" -a "x$version" = "x-1" ]; then
    echo "Must give a version number or a VERSION file!"
    exit 1
fi

# If a version number was given, use it.
if [ "x$version" != "x-1" ]; then
    ver_num="$version"
# Otherwise, read the version number from the top of $ver_file.
else
    if [ ! -r "$ver_file" ]; then
        echo "ERROR: Could not read $ver_file"
        exit 1
    fi

    ver_line=`head -1 $ver_file`
    ver_num=`echo $ver_line |  sed -e 's/^\(.*\) @.*/\1/'`
fi

# If a branch name was given, include it.
if [ "x$branch" != "x-1" ]; then
    string="v$ver_num ($subsystem) $branch $date"
else
    string="v$ver_num ($subsystem) $date"
fi

# Make sure we can read from $input.
if [ ! -r "$input" ]; then
    echo "ERROR: Could not read $input_file"
    exit 1
fi

# Extract the major, minor, and patch numbers and reconstruct them as a single
# integer.
major=`echo $ver_num | sed -e 's/^\([0-9][0-9]*\)\..*/\1/'`
minor=`echo $ver_num | sed -e 's/^.*\.\([0-9][0-9]*\)\..*$/\1/'`
patch=`echo $ver_num | sed -e 's/^.*\.\([0-9][0-9]*\)$/\1/'`
number=`printf "%03d%03d%03dul" $major $minor $patch | sed -e 's/^[0]*\([^0].*\)$/\1/'`

# Create the temporary ouptut file.
cat $input | sed -e "s/@VER_STRING@/\"$string\"/"	\
                 -e "s/@VER_NUMBER@/$number/"		\
           > ${output}.temp

# If the output file already exists, compare it with the temporary version.
# If the two are the same, there is no need to overwrite $output.
if [ -r "$output" ]; then
    if cmp -s "$output" "${output}.temp" 2>/dev/null ; then
        echo "$output is unchanged"
        rm -f "${output}.temp"
    else
        mv -f "${output}.temp" "$output"
    fi
else
    mv -f "${output}.temp" "$output"
fi

exit 0
