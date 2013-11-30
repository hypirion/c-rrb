/*
 * Copyright (c) 2013 Jean Niklas L'orange. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#define _DYNAMIC 0
#define GC_LINUX_THREADS
#ifndef _REENTRANT
#define _REENTRANT 1
#endif

#include <gc/gc.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <rrb.h>
#include "interval.h"
#include "substr_contains.h"

typedef struct {
  char *buffer;
  uint32_t file_size;
  uint32_t own_tid;
  uint32_t thread_count;
  RRB **intervals;
  pthread_barrier_t *barriers;
} LineSplitArgs;

typedef struct {
  char *buffer;
  char *search_term;
  uint32_t own_tid;
  uint32_t thread_count;
  RRB *lines;
  RRB **intervals;
  pthread_barrier_t *barriers;
} FilterArgs;

static void* split_to_lines(void *void_input);
static void* filter_by_term(void *void_input);
static inline void concatenate_rrbs(uint32_t own_tid, uint32_t thread_count,
                                    RRB **intervals,
                                    pthread_barrier_t *barriers);

int main(int argc, char *argv[]) {
  GC_INIT();
  // CLI argument parsing
  if (argc != 4) {
    fprintf(stderr, "Expected 3 arguments, got %d\nExiting...\n", argc - 1);
    exit(1);
  }
  else {
    char *end;
    uint32_t thread_count = (uint32_t) strtol(argv[1], &end, 10);
    if (*end) {
      fprintf(stderr, "Error, expects first argument to be a power of two,"
              " was '%s'.\n", argv[1]);
      exit(1);
    }
    if (!(thread_count && !(thread_count & (thread_count - 1)))) {
      fprintf(stderr, "Error, first argument must be a power of two, not %d.\n",
              thread_count);
      exit(1);
    }
    fprintf(stderr, "Looking for '%s' in file %s using %d threads...\n",
            argv[2], argv[3], thread_count);

    char *search_term = argv[2];

    FILE *fp;
    long file_size;
    char *buffer;
    fp = fopen(argv[3], "rb");
    if (!fp) {
      perror(argv[1]);
      exit(1);
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    buffer = malloc(file_size + 1);
    if (!buffer) {
      fclose(fp);
      fprintf(stderr, "Cannot allocate buffer for file (probably too large).\n");
      exit(1);
    }
    buffer[file_size] = 0;

    if (1 != fread(buffer, file_size, 1, fp)) {
      fclose(fp);
      free(buffer);
      fprintf(stderr, "Entire read failed.\n");
      exit(1);
    }
    else {
      fclose(fp);
    }

    // Find all lines
    pthread_t *tid = malloc(thread_count * sizeof(pthread_t));
    LineSplitArgs *lsa = malloc(thread_count * sizeof(LineSplitArgs));
    pthread_barrier_t *barriers = malloc(thread_count * sizeof(pthread_barrier_t));
    RRB **intervals = (RRB **) GC_MALLOC(thread_count * sizeof(RRB *));

    for (uint32_t i = 0; i < thread_count; i++) {
      pthread_barrier_init(&barriers[i], NULL, 2);
    }

    for (uint32_t i = 0; i < thread_count; i++) {
      LineSplitArgs arguments =
        {.buffer = buffer, .file_size = (uint32_t) file_size,
         .own_tid = i, .thread_count = thread_count,
         .intervals = intervals, .barriers = barriers};
      lsa[i] = arguments;
      pthread_create(&tid[i], NULL, &split_to_lines, (void *) &lsa[i]);
    }

    for (uint32_t i = 0; i < thread_count; i++) {
      pthread_join(tid[i], NULL);
    }
    free(lsa);
    RRB *lines = intervals[0]; // should be okay?
    // Found all lines, now onto searching in each list

    fprintf(stderr, "%d newlines\n", rrb_count(intervals[0]));

    FilterArgs *fa = malloc(thread_count * sizeof(FilterArgs));
    // Reuse barrier and intervals array

    for (uint32_t i = 0; i < thread_count; i++) {
      pthread_barrier_init(&barriers[i], NULL, 2);
    }

    for (uint32_t i = 0; i < thread_count; i++) {
      FilterArgs arguments =
        {.buffer = buffer, .search_term = search_term,
         .lines = lines,
         .own_tid = i, .thread_count = thread_count,
         .intervals = intervals, .barriers = barriers};
      fa[i] = arguments;
      pthread_create(&tid[i], NULL, &filter_by_term, (void *) &fa[i]);
    }

    for (uint32_t i = 0; i < thread_count; i++) {
      pthread_join(tid[i], NULL);
    }

    fprintf(stderr, "%d hits\n", rrb_count(intervals[0]));

    free(barriers);
    free(buffer);
    exit(0);
  }

}

void* split_to_lines(void *void_input) {
  LineSplitArgs *lsa = (LineSplitArgs *) void_input;
  const char *buffer = lsa->buffer;
  const uint32_t own_tid = lsa->own_tid;
  const uint32_t file_size = lsa->file_size;
  const uint32_t thread_count = lsa->thread_count;
  RRB **intervals = lsa->intervals;
  pthread_barrier_t *barriers = lsa->barriers;

  // calculate the interval to compute for
  uint32_t partition_size = file_size / thread_count;
  uint32_t from = partition_size * own_tid;
  uint32_t to = partition_size * (own_tid + 1);
  if (own_tid + 1 == thread_count) {
    to = file_size;
  }

  // find the lines
  RRB *lines = rrb_create();

  // rewind to start of line
  uint32_t line_start = from;
  while (0 < line_start && lsa->buffer[line_start] != '\n') {
    line_start--;
  }
  // find and collect lines
  for (uint32_t i = from; i < to; i++) {
    if (buffer[i] == '\n') {
      Interval interval = {.from = line_start, .to = i};
      lines = rrb_push(lines, (void *) interval_to_uint64_t(interval));
      line_start = i + 1;
    }
  }

  intervals[own_tid] = lines;
  concatenate_rrbs(own_tid, thread_count, intervals, barriers);
  pthread_barrier_destroy(&barriers[own_tid]);
  return 0;
}

void* filter_by_term(void *void_input) {
  FilterArgs *fa = (FilterArgs *) void_input;
  const char *buffer = fa->buffer;
  const char *search_term = fa->search_term;
  const uint32_t own_tid = fa->own_tid;
  const uint32_t thread_count = fa->thread_count;
  RRB *lines = fa->lines;
  RRB **intervals = fa->intervals;
  pthread_barrier_t *barriers = fa->barriers;

  // calculate the lines to compute for
  uint32_t partition_size = rrb_count(lines) / thread_count;
  uint32_t from = partition_size * own_tid;
  uint32_t to = partition_size * (own_tid + 1);
  if (own_tid + 1 == thread_count) {
    to = rrb_count(lines);
  }

  // find all lines containing the search term
  RRB *contained_lines = rrb_create();

  for (uint32_t line_idx = from; line_idx < to; line_idx++) {
    Interval line = (Interval) uint64_t_to_interval((uint64_t) rrb_nth(lines, line_idx));
    if (substr_contains(&buffer[line.from], line.to - line.from, search_term)) {
      contained_lines = rrb_push(contained_lines, (void *) interval_to_uint64_t(line));
    }
  }

  intervals[own_tid] = contained_lines;
  concatenate_rrbs(own_tid, thread_count, intervals, barriers);
  pthread_barrier_destroy(&barriers[own_tid]);

  return 0;
}


static inline void concatenate_rrbs(uint32_t own_tid, uint32_t thread_count,
                                    RRB **intervals,
                                    pthread_barrier_t *barriers) {
  // barrier before merging result, to avoid race conditions
  uint32_t sync_mask = (uint32_t) 1;
  while ((own_tid | sync_mask) != own_tid) {
    uint32_t sync_tid = own_tid | sync_mask;
    if (thread_count <= sync_tid) {
      // jump out here, finished.
      return;
    }
    pthread_barrier_wait(&barriers[sync_tid]);
    // concatenate data
    intervals[own_tid] = rrb_concat(intervals[own_tid], intervals[sync_tid]);
    sync_mask = sync_mask << 1;
  }
  pthread_barrier_wait(&barriers[own_tid]);
}
