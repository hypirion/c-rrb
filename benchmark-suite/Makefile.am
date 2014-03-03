AM_CPPFLAGS = -I$(srcdir)/../src
AM_CFLAGS = $(DEBUG_VARS) -pthread
LDADD = $(GCLIB) ../src/librrb.la -lpthread

all:

benchmark: pgrep_rrb grep_array pgrep_array pgrep_dummy pgrep_mem_array \
					 pgrep_mem_rrb

EXTRA_PROGRAMS =

EXTRA_PROGRAMS += grep_array
grep_array_SOURCES = grep_array.c interval.c

EXTRA_PROGRAMS += pgrep_rrb
pgrep_rrb_SOURCES = pgrep_rrb.c interval.c

EXTRA_PROGRAMS += pgrep_mem_rrb
pgrep_mem_rrb_SOURCES = pgrep_mem_rrb.c interval.c

EXTRA_PROGRAMS += pgrep_array
pgrep_array_SOURCES = pgrep_array.c interval.c

EXTRA_PROGRAMS += pgrep_mem_array
pgrep_mem_array_SOURCES = pgrep_mem_array.c interval.c

EXTRA_PROGRAMS += pgrep_dummy
pgrep_dummy_SOURCES = pgrep_dummy.c interval.c
