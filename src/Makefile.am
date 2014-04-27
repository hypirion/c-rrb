lib_LTLIBRARIES = librrb.la
include_HEADERS = rrb.h

librrb_la_LIBADD = $(THREADLIB)
librrb_la_SOURCES = rrb.c
librrb_la_CFLAGS = $(DEBUG_VARS)

rrb.c: rrb_transients.c rrb.h rrb_alloc.h decrement.h unroll.h
