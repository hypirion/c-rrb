AC_INIT([rrb], [0.1.0-SNAPSHOT], [jeannikl@hypirion.com])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_PROG_CC([clang gcc cc])

m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AC_PROG_RANLIB

dnl Find the build directory and the source directory
top_builddir_absolute=`pwd`
AC_SUBST(top_builddir_absolute)
top_srcdir_absolute=`(cd $srcdir && pwd)`
AC_SUBST(top_srcdir_absolute)

dnl Config scripts
AC_DEFUN([RRB_CONFIG_SCRIPT],[AC_CONFIG_FILES([$1],[chmod +x $1])])

dnl Check for programs needed for config scripts
AC_CHECK_PROG(FIND_CHECK,find,yes)
if test "x$FIND_CHECK" != "xyes" ; then
    AC_MSG_ERROR([Please install 'find' before installing.])
fi

dnl Setup debug variables for c
case $CC in
   *clang*) DEBUG_VARS='-Weverything -Wno-cast-align' ;;
   *gcc*)   DEBUG_VARS='-Wall' ;;
   *)       DEBUG_VARS='' ::
esac
AC_SUBST([DEBUG_VARS])

AC_PROG_CC_STDC

AC_CHECK_LIB([gc], [GC_malloc], [GCLIB=-lgc],
             AC_MSG_ERROR([Please install libgc in order to compile librrb.]))
AC_SUBST([GCLIB])

dnl Check for header files here.
AC_CHECK_HEADERS([stdio.h])
AC_CHECK_HEADERS([stdint.h])
AC_CHECK_HEADERS([stdlib.h])
AC_CHECK_HEADERS([string.h])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])

RRB_CONFIG_SCRIPT([check-rrb])

AC_OUTPUT
