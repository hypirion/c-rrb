lib_LTLIBRARIES = librrb.la
include_HEADERS = rrb.h

librrb_la_SOURCES = rrb.c
librrb_la_CPPFLAGS = -DRRB_DEBUG -DRRB_BITS=$(RRB_BITS)
librrb_la_CFLAGS = $(DEBUG_VARS)

# Handling direct append insertion
RRB_PUSH_DEP=

if DIRECT_APPEND
RRB_PUSH_DEP += generated/rrb_push.c
endif

rrb.c: generated/rrb_nth.c generated/rrb_update.c $(RRB_PUSH_DEP)

generated/%.c: generated/%.py
	python $< $(RRB_BITS) > $@

clean-local:
	-rm -f generated/rrb_nth.c
	-rm -f generated/rrb_update.c
	-rm -f generated/rrb_push.c
