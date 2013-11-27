AM_CPPFLAGS = -I$(srcdir)/../src
AM_CFLAGS = $(DEBUG_VARS) -pthread
LDADD = $(GCLIB) ../src/librrb.la -lpthread

benchmark: pgrep_rrb grep_array pgrep_array

EXTRA_PROGRAMS =

EXTRA_PROGRAMS += pgrep_rrb
pgrep_rrb_SOURCES = pgrep_rrb.c

EXTRA_PROGRAMS += grep_array
grep_array_SOURCES = grep_array.c interval.c

EXTRA_PROGRAMS += pgrep_array
pgrep_array_SOURCES = pgrep_array.c interval.c
