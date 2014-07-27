AM_CPPFLAGS = -I$(srcdir)/../src
AM_CFLAGS = $(DEBUG_VARS)
LDADD = $(GCLIB) ../src/librrb.la
check_PROGRAMS =
TESTS =

check_PROGRAMS += test_concat
TESTS += test_concat
test_concat_SOURCES = test_concat.c test.h

check_PROGRAMS += test_push
TESTS += test_push
test_push_SOURCES = test_push.c test.h

check_PROGRAMS += test_peek
TESTS += test_peek
test_peek_SOURCES = test_peek.c test.h

check_PROGRAMS += test_pop
TESTS += test_pop
test_pop_SOURCES = test_pop.c test.h

check_PROGRAMS += test_update
TESTS += test_update
test_update_SOURCES = test_update.c test.h

check_PROGRAMS += test_slice
TESTS += test_slice
test_slice_SOURCES = test_slice.c test.h

check_PROGRAMS += test_catslice
TESTS += test_catslice
test_catslice_SOURCES = test_catslice.c test.h

check_PROGRAMS += test_fibocat
TESTS += test_fibocat
test_fibocat_SOURCES = test_fibocat.c test.h

if TRANSIENTS
transient_check_programs = test_transient_push test_transient_push_2 \
													 test_transient_update test_transient_pop
transient_tests = test_transient_push test_transient_push_2 test_transient_update \
                  test_transient_pop

test_transient_push_SOURCES = test_transient_push.c test.h
test_transient_push_2_SOURCES = test_transient_push_2.c test.h
test_transient_update_SOURCES = test_transient_update.c test.h
test_transient_pop_SOURCES = test_transient_pop.c test.h
endif

check_PROGRAMS += $(transient_check_programs)
TESTS += $(transient_tests)
