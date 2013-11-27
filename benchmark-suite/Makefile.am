AM_CPPFLAGS = -I$(srcdir)/../src
AM_CFLAGS = $(DEBUG_VARS)
LDADD = $(GCLIB) ../src/librrb.la

benchmark: pgrep_rrb pgrep_array

EXTRA_PROGRAMS =

EXTRA_PROGRAMS += pgrep_rrb
pgrep_rrb_SOURCES = pgrep_rrb.c

EXTRA_PROGRAMS += pgrep_array
pgrep_array_SOURCES = pgrep_array.c
