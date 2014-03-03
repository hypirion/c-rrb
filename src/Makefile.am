lib_LTLIBRARIES = librrb.la
include_HEADERS = rrb.h

librrb_la_SOURCES = rrb.c
librrb_la_CPPFLAGS = -DRRB_DEBUG -DRRB_BITS=$(RRB_BITS)
librrb_la_CFLAGS = $(DEBUG_VARS)

rrb.c: generated/rrb_nth.c

generated/rrb_nth.c:
	python generated/rrb_nth.py $(RRB_BITS) > generated/rrb_nth.c

clean-local:
	-rm -rf generated/rrb_nth.c
