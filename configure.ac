AC_INIT([rrb], [0.1.0-SNAPSHOT], [jeannikl@hypirion.com])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_PROG_CC([clang gcc cc])

dnl Setup debug variables
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
AC_OUTPUT
