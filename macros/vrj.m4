dnl ************ <auto-copyright.pl BEGIN do not edit this line> ************
dnl
dnl VR Juggler is (C) Copyright 1998-2002 by Iowa State University
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
dnl -----------------------------------------------------------------
dnl File:          $RCSfile$
dnl Date modified: $Date$
dnl Version:       $Revision$
dnl -----------------------------------------------------------------
dnl
dnl ************* <auto-copyright.pl END do not edit this line> *************

dnl ---------------------------------------------------------------------------
dnl VRJUGGLER_PATH([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, MODULES]]]])
dnl
dnl Test for VR Juggler and then define the following variables:
dnl     VRJ_CXXFLAGS
dnl     VRJ_CXXFLAGS_ALL
dnl     VRJ_LIBS_CC
dnl     VRJ_LIBS_LD
dnl     VRJ_LIBS_STATIC_CC
dnl     VRJ_LIBS_STATIC_LD
dnl     VRJ_EXTRA_LIBS_CC
dnl     VRJ_EXTRA_LIBS_LD
dnl ---------------------------------------------------------------------------
AC_DEFUN(VRJUGGLER_PATH,
[
    dnl Get the cflags and libraries from the vrjuggler-config script
    AC_ARG_WITH(vrj-prefix,
                [  --with-vrj-prefix=<PATH> Prefix where VR Juggler is
                          installed (optional)            [No default]],
                vrj_config_prefix="$withval", vrj_config_prefix="")
    AC_ARG_WITH(vrj-exec-prefix,
                [  --with-vrj-exec-prefix=<PATH>
                          Exec prefix where VR Juggler is
                          installed (optional)            [No default]],
                vrj_config_exec_prefix="$withval", vrj_config_exec_prefix="")
dnl    AC_ARG_ENABLE(vrjtest,
dnl                  [  --disable-vrjtest       Do not try to compile and run a
dnl                          test VR Juggler program], , enable_vrjtest=yes)

    if test "x$vrj_config_exec_prefix" != "x" ; then
        vrj_config_args="$vrj_config_args --exec-prefix=$vrj_config_exec_prefix"

        if test x${VRJ_CONFIG+set} != xset ; then
            VRJ_CONFIG="$vrj_config_exec_prefix/bin/vrjuggler-config"
        fi
    fi

    if test "x$vrj_config_prefix" != "x" ; then
        vrj_config_args="$vrj_config_args --prefix=$vrj_config_prefix"

        if test x${VRJ_CONFIG+set} != xset ; then
            VRJ_CONFIG="$vrj_config_prefix/bin/vrjuggler-config"
        fi
    fi

    if test "x$VJ_BASE_DIR" != "x" ; then
        vrj_config_args="$vrj_config_args --prefix=$VJ_BASE_DIR"

        if test x${VRJ_CONFIG+set} != xset ; then
            VRJ_CONFIG="$VJ_BASE_DIR/bin/vrjuggler-config"
        fi
    fi

    AC_PATH_PROG(VRJ_CONFIG, vrjuggler-config, no)
    min_vrj_version=ifelse([$1], ,0.0.1,$1)

    dnl Do a sanity check to ensure that $VRJ_CONFIG actually works.
    if ! (eval $VRJ_CONFIG --cxxflags >/dev/null 2>&1) 2>&1 ; then
        VRJ_CONFIG='no'
    fi

    no_vrj=''
    if test "x$VRJ_CONFIG" = "xno" ; then
        no_vrj=yes
    else
dnl        VRJ_LIBS="`$VRJ_CONFIG $vrj_config_args --libs $ABI` $VRJ_EXTRA_LIBS"
dnl        VRJ_LIBS_STATIC="`$VRJ_CONFIG $vrj_config_args --libs $ABI --static` $VRJ_EXTRA_LIBS"

        VRJ_CXXFLAGS=`$VRJ_CONFIG $vrj_config_args --cxxflags $ABI`
        VRJ_CXXFLAGS_ALL=`$VRJ_CONFIG $vrj_config_args --cxxflags $ABI --all`
        VRJ_LIBS_LD="`$VRJ_CONFIG $vrj_config_args --linker --libs $ABI`"
        VRJ_LIBS_STATIC_LD="`$VRJ_CONFIG $vrj_config_args --linker --libs $ABI --static`"
        VRJ_LIBS_CC="`$VRJ_CONFIG $vrj_config_args --libs $ABI`"
        VRJ_LIBS_STATIC_CC="`$VRJ_CONFIG $vrj_config_args --libs $ABI --static`"
        VRJ_EXTRA_LIBS_CC=`$VRJ_CONFIG $vrj_config_args --extra-libs $ABI`
        VRJ_EXTRA_LIBS_LD=`$VRJ_CONFIG $vrj_config_args --extra-libs $ABI --linker`
        VRJ_EXTRA_LIBS_ALL_CC=`$VRJ_CONFIG $vrj_config_args --extra-libs $ABI --all`
        VRJ_EXTRA_LIBS_ALL_LD=`$VRJ_CONFIG $vrj_config_args --extra-libs $ABI --all --linker`
        VRJ_VERSION=`$VRJ_CONFIG --version`

        AC_MSG_CHECKING([whether VR Juggler version is >= $min_vrj_version])
        AC_MSG_RESULT([$VRJ_VERSION])
        DPP_VERSION_CHECK([$VRJ_VERSION], [$min_vrj_version], $2, $3)
    fi

    if test "x$no_vrj" != x ; then
        if test "$VRJ_CONFIG" = "no" ; then
            echo "*** The vrjuggler-config script installed by VR Juggler could not be found"
            echo "*** If VR Juggler was installed in PREFIX, make sure PREFIX/bin is in"
            echo "*** your path, or set the VRJ_CONFIG environment variable to the"
            echo "*** full path to vrjuggler-config."
        fi
        VRJ_CXXFLAGS=""
        VRJ_CXXFLAGS_ALL=""
        VRJ_LIBS_CC=""
        VRJ_LIBS_LD=""
        VRJ_LIBS_STATIC_CC=""
        VRJ_LIBS_STATIC_LD=""
        VRJ_EXTRA_LIBS_CC=""
        VRJ_EXTRA_LIBS_LD=""
        VRJ_EXTRA_LIBS_ALL_CC=""
        VRJ_EXTRA_LIBS_ALL_LD=""
        VRJ_VERSION="-1"
        ifelse([$3], , :, [$3])
    fi

    AC_SUBST(VRJ_CXXFLAGS)
    AC_SUBST(VRJ_CXXFLAGS_ALL)
    AC_SUBST(VRJ_LIBS_CC)
    AC_SUBST(VRJ_LIBS_LD)
    AC_SUBST(VRJ_LIBS_STATIC_CC)
    AC_SUBST(VRJ_LIBS_STATIC_LD)
    AC_SUBST(VRJ_EXTRA_LIBS_CC)
    AC_SUBST(VRJ_EXTRA_LIBS_LD)
    AC_SUBST(VRJ_EXTRA_LIBS_ALL_CC)
    AC_SUBST(VRJ_EXTRA_LIBS_ALL_LD)
    AC_SUBST(VRJ_VERSION)
])
