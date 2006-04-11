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

# -----------------------------------------------------------------------------
# This include file <VjConfig.mk> defines the list of .java files that are
# built into Java bytecode with javac.
# -----------------------------------------------------------------------------
# Variables defined here are:
#
# VJCONFIG_SOURCES - The list of .java files to build
# VJCONFIG_OBJS    - The list of .class files that will be created.
# -----------------------------------------------------------------------------

VJCONFIG_SOURCES = VjConfig/ChunkDependEntry.java       \
		   VjConfig/ConfigChunkDB.java		\
                   VjConfig/ConfigChunk.java		\
                   VjConfig/Property.java		\
                   VjConfig/Property.java		\
                   VjConfig/ChunkDescDB.java		\
                   VjConfig/ChunkDesc.java		\
                   VjConfig/DescEnum.java		\
                   VjConfig/PropertyDesc.java		\
                   VjConfig/ValType.java		\
                   VjConfig/VarValue.java		\
                   VjConfig/VarValueException.java	\
                   VjConfig/ConfigStreamTokenizer.java  \
		   VjConfig/ChunkFactory.java		\
		   VjConfig/ChunkDBEvent.java		\
		   VjConfig/ChunkDBListener.java	\
		   VjConfig/DescDBEvent.java		\
		   VjConfig/DescDBListener.java

VJCONFIG_OBJS = ${VJCONFIG_SOURCES:.java=.class}
