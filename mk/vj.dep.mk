# -----------------------------------------------------------------------------
# $Id$
#
# This include file <vj.dep.mk> handles source code dependencies.  It
# generates makefiles corresponding to each source file (ending in .c or .cpp)
# that have the dependencies for that source file.  The generated file can
# then be included in the Makefile for the directory.
# -----------------------------------------------------------------------------
# The Makefile including this file must define the following variables:
#
# C_COMPILE       - The compiler command for C files.
# CXX_COMPILE     - The compiler command for C++ files.
# DEP_GEN_FLAG    - The flag passed to the compiler to generate dependencies.
# DEPEND_FILES    - The list of dependency makefiles to be generated.
# OBJDIR          - The directory to which the object file(s) will be written.
# OBJ_FILE_SUFFIX - Suffix for object file names (e.g., "o" or "obj").
#
# Example:
#         srcdir = /usr/src/proj1
#         OBJDIR = /usr/obj/proj1
#   DEP_GEN_FLAG = -M
#   DEPEND_FILES = file1.d file2.d file3.d
#
# With these settings, the source code comes from /usr/src/proj1 and the
# object files go into /usr/obj/proj1.
# -----------------------------------------------------------------------------

# Conditionally set ${OBJDIR} just to be safe.
ifndef OBJDIR
    OBJDIR	= .
endif

ifdef DO_CLEANDEPEND
    CLEAN_FILES	+= ${DEPEND_FILES}
endif

# These expressions reformat the output from the dependency text to be of the
# form:
#
#     ${OBJDIR}/file1.o file1.d : ...
#
# where file1 is the value in $* and file1.d is $@.  The first handles output
# from the C and C++ compilers which prints only the object file to be
# created ($*).  The second handles output from makedepend(1) which includes
# the path leading to the source file in the object file name.
_CC_SED_EXP	= '\''s/\($*\)\.${OBJ_FILE_SUFFIX}[ :]*/$${OBJDIR}\/\1.${OBJ_FILE_SUFFIX} $@: /g'\''
_MKDEP_SED_EXP	= '\''s/.*\($*\)\.${OBJ_FILE_SUFFIX}[ :]*/$${OBJDIR}\/\1.${OBJ_FILE_SUFFIX} $@: /g'\''

%.d: %.c
	@echo "Updating dependency file $@ ..."
ifeq (${CC}, cl)
	@${SHELL} -ec 'makedepend -f- -o.${OBJ_FILE_SUFFIX}		\
                       ${DEPENDFLAGS} -- ${DEPEND_EXTRAS} -- $< |	\
                       sed ${_MKDEP_SED_EXP} > $@ ; [ -s $@ ] || rm -f $@'
else
	@${SHELL} -ec '${C_COMPILE} ${DEP_GEN_FLAG} $< |		\
                       sed ${_CC_SED_EXP} > $@ ; [ -s $@ ] || rm -f $@'
endif

%.d: %.cpp
	@echo "Updating dependency file $@ ..."
ifeq (${CC}, cl)
	@${SHELL} -ec 'makedepend -f- -o.${OBJ_FILE_SUFFIX}		\
                       ${DEPENDFLAGS} -- ${DEPEND_EXTRAS} -- $< |	\
                       sed ${_MKDEP_SED_EXP} > $@ ; [ -s $@ ] || rm -f $@'
else
	@${SHELL} -ec '${CXX_COMPILE} ${DEP_GEN_FLAG} $< |		\
                       sed ${_CC_SED_EXP} > $@ ; [ -s $@ ] || rm -f $@'
endif
