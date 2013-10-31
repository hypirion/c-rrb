bin_PROGRAMS = test
lib_LIBRARIES = librrb.a
include_HEADERS = rrb.h

test_SOURCES = main.c rrb.c
test_LDADD = $(GCLIB)
test_CPPFLAGS = -DRRB_DEBUG
test_CFLAGS = $(DEBUG_VARS)

librrb_a_SOURCES = rrb.c
librrb_a_CPPFLAGS = -DRRB_DEBUG
librrb_a_CFLAGS = $(DEBUG_VARS)
