dnl ************* <auto-copyright.pl BEGIN do not edit this line> *************
dnl
dnl VR Juggler is (C) Copyright 1998-2006 by Iowa State University
dnl
dnl Original Authors:
dnl   Allen Bierbaum, Christopher Just,
dnl   Patrick Hartling, Kevin Meinert,
dnl   Carolina Cruz-Neira, Albert Baker
dnl
dnl This library is free software; you can redistribute it and/or
dnl modify it under the terms of the GNU Library General Public
dnl License as published by the Free Software Foundation; either
dnl version 2 of the License, or (at your option) any later version.
dnl
dnl This library is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl Library General Public License for more details.
dnl
dnl You should have received a copy of the GNU Library General Public
dnl License along with this library; if not, write to the
dnl Free Software Foundation, Inc., 59 Temple Place - Suite 330,
dnl Boston, MA 02111-1307, USA.
dnl
dnl ************** <auto-copyright.pl END do not edit this line> **************

dnl ---------------------------------------------------------------------------
dnl NOTE: This should not be called by external code.
dnl ---------------------------------------------------------------------------
AC_DEFUN([_JCCL_PATH_SETUP],
[
    dnl Get the cflags and libraries from the flagpoll script
    AC_ARG_WITH(jccl-meta-file,
                [  --with-jccl-meta-file=<PATH>      Flagpoll metadata file
                          for JCCL                       [No default]],
                jccl_config_prefix="$withval", jccl_config_prefix="")

    dnl See if the user specified where to find jccl
    dnl if they didn't take a guess for them
    if test "x$jccl_meta_file" != "x" ; then
        jccl_flagpoll_args="--from-file=$jccl_meta_file"
    else
        jccl_flagpoll_args="--from-file=$instlinks/share/flagpoll/jccl.fpc"
    fi

    jccl_flagpoll_args="jccl $jccl_flagpoll_args --no-deps"
    AM_PATH_FLAGPOLL([0.7.0], , [AC_MSG_ERROR(*** Flagpoll required for Jackal Flags ***)])

    dnl Do a sanity check to ensure that $FLAGPOLL actually works.
    if ! (eval $FLAGPOLL --help >/dev/null 2>&1) 2>&1 ; then
        FLAGPOLL='no'
        echo "*** Flagpoll is required to build jackal."
        echo "*** Please check that the PATH variable is set to "
        echo "*** include the proper path to flagpoll."
    fi
])

dnl ---------------------------------------------------------------------------
dnl _JCCL_VERSION_CHECK(minimum-version, [action-if-found [, action-if-not-found]])
dnl NOTE: This should not be called by external code.
dnl ---------------------------------------------------------------------------
AC_DEFUN([_JCCL_VERSION_CHECK],
[
   DPP_PREREQ([2.0.1])

   AC_REQUIRE([_JCCL_PATH_SETUP])

   if test "x$FLAGPOLL" = "xno" ; then
      ifelse([$3], , :, [$3])
   else
      JCCL_VERSION=`$FLAGPOLL $jccl_flagpoll_args --modversion`

      min_jccl_version=ifelse([$1], , 0.0.1, $1)
      DPP_VERSION_CHECK_MSG_NO_CACHE([JCCL], [$JCCL_VERSION],
                                     [$min_jccl_version], [$2], [$3])
   fi
])

dnl ---------------------------------------------------------------------------
dnl JCCL_PATH_CXX([minimum-version, [action-if-found [, action-if-not-found]]])
dnl
dnl Tests for JCCL C++ API and then defines the following variables:
dnl     JCCL_CXXFLAGS
dnl     JCCL_LIBS
dnl     JCCL_LIBS_STATIC
dnl     JCCL_PROF_LIBS
dnl     JCCL_PROF_LIBS_STATIC
dnl ---------------------------------------------------------------------------
AC_DEFUN([JCCL_PATH_CXX],
[
   AC_REQUIRE([_JCCL_PATH_SETUP])

   JCCL_CXXFLAGS=""
   JCCL_LIBS=""
   JCCL_LIBS_STATIC=""
   JCCL_PROF_LIBS=""
   JCCL_PROF_LIBS_STATIC=""

   if test "x$FLAGPOLL" = "xno" ; then
      ifelse([$3], , :, [$3])
   else
      AC_MSG_CHECKING([whether JCCL C++ API is available])
      has_cxx=`$FLAGPOLL $jccl_flagpoll_args --get-cxx-api-available`

      if test "x$has_cxx" = "xY" ; then
         AC_MSG_RESULT([yes])
         if test "x$JCCL_VERSION" = "x" ; then
            _JCCL_VERSION_CHECK($1, [jccl_version_okay='yes'],
                                 [jccl_version_okay='no'
                                  $3])
         fi

         if test "x$jccl_version_okay" = "xyes" ; then
            JCCL_CXXFLAGS=`$FLAGPOLL $jccl_flagpoll_args --cflags `
            JCCL_LIBS=`$FLAGPOLL $jccl_flagpoll_args --get-libs`
            JCCL_EXTRA_LIBS=`$FLAGPOLL $jccl_flagpoll_args --get-extra-libs`
            JCCL_PROF_LIBS=`$FLAGPOLL $jccl_flagpoll_args --get-profiled-libs`
            JCCL_LIBS_STATIC=`$FLAGPOLL $jccl_flagpoll_args --get-static-libs`
            JCCL_PROF_LIBS_STATIC=`$FLAGPOLL $jccl_flagpoll_args --get-profiled-static-libs`

            ifelse([$2], , :, [$2])
         fi
      else
         AC_MSG_RESULT([no])
         ifelse([$3], , :, [$3])
      fi
   fi

   AC_SUBST([JCCL_CXXFLAGS])
   AC_SUBST([JCCL_LIBS])
   AC_SUBST([JCCL_PROF_LIBS])
   AC_SUBST([JCCL_LIBS_STATIC])
   AC_SUBST([JCCL_PROF_LIBS_STATIC])
   AC_SUBST([JCCL_EXTRA_LIBS])
])

dnl ---------------------------------------------------------------------------
dnl JCCL_PATH_JAVA([minimum-version, [action-if-found [, action-if-not-found]]])
dnl
dnl Tests for JCCL Java API and then defines the following variables:
dnl     JCCL_JARS
dnl ---------------------------------------------------------------------------
AC_DEFUN([JCCL_PATH_JAVA],
[
   AC_REQUIRE([_JCCL_PATH_SETUP])

   JCCL_JARS=''

   if test "x$FLAGPOLL" = "xno" ; then
      ifelse([$3], , :, [$3])
   else
      AC_MSG_CHECKING([whether JCCL Java API is available])
      has_java=`$FLAGPOLL $jccl_flagpoll_args --get-java-api-available`

      if test "x$has_java" = "xY" ; then
         AC_MSG_RESULT([yes])
         if test "x$JCCL_VERSION" = "x" ; then
            _JCCL_VERSION_CHECK($1, [jccl_version_okay='yes'],
                                 [jccl_version_okay='no'
                                  $3])
         fi

         if test "x$jccl_version_okay" = "xyes" ; then
            JCCL_JARS="`$FLAGPOLL $jccl_flagpoll_args --get-jars`"

            ifelse([$2], , :, [$2])
         fi
      else
         AC_MSG_RESULT([no])
         ifelse([$3], , :, [$3])
      fi
   fi

   AC_SUBST([JCCL_JARS])
])

dnl ---------------------------------------------------------------------------
dnl JCCL_PATH([minimum-version, [action-if-found [, action-if-not-found]]])
dnl
dnl Tests for JCCL C++ and Java APIs and then defines the following
dnl variables:
dnl     JCCL_CXXFLAGS
dnl     JCCL_LIBS
dnl     JCCL_LIBS_STATIC
dnl     JCCL_PROF_LIBS
dnl     JCCL_PROF_LIBS_STATIC
dnl     JCCL_JARS
dnl ---------------------------------------------------------------------------
AC_DEFUN([JCCL_PATH],
[
   JCCL_PATH_CXX($1, [jccl_have_cxx='yes'], $3)

   if test "x$jccl_have_cxx" = "xyes" ; then
      JCCL_PATH_JAVA($1, $2, $3)
   fi
])
