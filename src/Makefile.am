lib_LTLIBRARIES = librrb.la
include_HEADERS = rrb.h

librrb_la_SOURCES = rrb.c
librrb_la_CPPFLAGS = -DRRB_DEBUG -DRRB_BITS=$(RRB_BITS)
librrb_la_CFLAGS = $(DEBUG_VARS)

rrb.c: generated/rrb_nth.c generated/rrb_update.c

generated/rrb_nth.c:
	python generated/rrb_nth.py $(RRB_BITS) > generated/rrb_nth.c

generated/rrb_update.c:
	python generated/rrb_update.py $(RRB_BITS) > generated/rrb_update.c

clean-local:
	-rm -f generated/rrb_nth.c
	-rm -f generated/rrb_update.c
