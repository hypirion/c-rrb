AC_INIT([rrb], [0.1.0-SNAPSHOT], [jeannikl@hypirion.com])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign -Wall])

AC_PROG_CC([clang gcc cc])
AM_PROG_AR
LT_INIT

dnl ----------------------------------------------------------------------------
dnl Find the build directory and the source directory

top_builddir_absolute=`pwd`
AC_SUBST(top_builddir_absolute)
top_srcdir_absolute=`(cd $srcdir && pwd)`
AC_SUBST(top_srcdir_absolute)

dnl ----------------------------------------------------------------------------
dnl Config scripts

AC_DEFUN([RRB_CONFIG_SCRIPT],[AC_CONFIG_FILES([$1],[chmod +x $1])])

dnl ----------------------------------------------------------------------------
dnl Setup debug variables for c

case $CC in
   *clang*) DEBUG_VARS='-Weverything -Wno-cast-align' ;;
   *gcc*)   DEBUG_VARS='-Wall' ;;
   *)       DEBUG_VARS='' ::
esac
AC_SUBST([DEBUG_VARS])

AC_PROG_CC_STDC

dnl ----------------------------------------------------------------------------
dnl Configs directly injected into rrb.h

AC_ARG_WITH([branching],
        [AS_HELP_STRING([--with-branching=bits : number of bits per branching])],
        [RRB_BITS=$withval],
        [RRB_BITS=5])

dnl Calculate max height of the rrb tree
[RRB_MAX_HEIGHT=1
RRB_MAX_HEIGHT_BITS=$RRB_BITS
while  [ "$RRB_MAX_HEIGHT_BITS" -lt 32 ]
do
  RRB_MAX_HEIGHT=`expr $RRB_MAX_HEIGHT + 1`
  RRB_MAX_HEIGHT_BITS=`expr $RRB_MAX_HEIGHT_BITS + $RRB_BITS`
done]

dnl RRB debug flag

AH_TEMPLATE([RRB_DEBUG],
        [Enable RRB-tree debugging functions.])

AC_ARG_ENABLE([rrb-debug],
[  --enable-rrb-debug    Enable rrb-tree debugging functions.],
[case "${enableval}" in
  yes) rrb_debug=true ;;
  no)  rrb_debug=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-rrb-debug]) ;;
esac],[rrb_debug=true])

if test x$rrb_debug = xtrue; then
   AC_DEFINE([RRB_DEBUG])
   AC_SUBST([RRB_DEBUG], true)
else
   AC_SUBST([RRB_DEBUG], false)
fi

dnl Number of bits in the rrb tree

AC_SUBST([RRB_BITS])

AC_DEFINE_UNQUOTED([RRB_BITS], [$RRB_BITS],
                               [The amount of bits used for a single node.])
AC_DEFINE_UNQUOTED([RRB_MAX_HEIGHT], [$RRB_MAX_HEIGHT],
                                     [The maximal height the RRB tree can have.])
AC_SUBST([RRB_MAX_HEIGHT], [$RRB_MAX_HEIGHT])
dnl ----------------------------------------------------------------------------
dnl Check that libraries exist

AC_CHECK_LIB([gc], [GC_malloc], [GCLIB=-lgc],
             AC_MSG_ERROR([Please install libgc in order to compile librrb.]))
AC_SUBST([GCLIB])

dnl ----------------------------------------------------------------------------
dnl Check for header files here.

AC_CHECK_HEADERS([stdio.h])
AC_CHECK_HEADERS([stdint.h])
AC_CHECK_HEADERS([stdlib.h])
AC_CHECK_HEADERS([string.h])
AC_CHECK_HEADERS([time.h])

AC_CONFIG_FILES([Makefile benchmark-suite/Makefile
                 test-suite/Makefile src/Makefile
                 src/rrb.h])

AC_OUTPUT
