AM_CPPFLAGS = -I$(srcdir)/../src
AM_CFLAGS = $(DEBUG_VARS)
LDADD = $(GCLIB) ../src/librrb.la
check_PROGRAMS =
TESTS =

check_PROGRAMS += test_no_cat_segfault
TESTS += test_no_cat_segfault
test_no_cat_segfault_SOURCES = test_no_cat_segfault.c

check_PROGRAMS += test_push
TESTS += test_push
test_push_SOURCES = test_push.c

