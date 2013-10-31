AM_CPPFLAGS = -I$(srcdir)/../src
check_PROGRAMS = test_push test
TESTS = test_push test

test_SOURCES = main.c
test_LDADD = $(GCLIB) ../src/librrb.la
test_CFLAGS = $(DEBUG_VARS)

test_push_SOURCES = test_push.c
test_push_LDADD = $(GCLIB) ../src/librrb.la
test_push_CFLAGS = $(DEBUG_VARS)
