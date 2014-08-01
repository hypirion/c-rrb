lib_LTLIBRARIES = librrb.la
include_HEADERS = rrb.h

librrb_la_LIBADD = $(THREADLIB)
librrb_la_SOURCES = rrb.c rrb_alloc.h rrb_transients.h rrb_thread.h
librrb_la_CFLAGS = $(DEBUG_VARS)

rrb.c: rrb_transients.h rrb.h rrb_alloc.h rrb_thread.h
rrb_alloc.h:
decrement.h:
unroll.h:
rrb_transients.h:
