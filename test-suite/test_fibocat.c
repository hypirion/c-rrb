/*
 * Copyright (c) 2013-2014 Jean Niklas L'orange. All rights reserved.
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

#include <gc/gc.h>
#include <stdio.h>
#include <stdlib.h>
#include "rrb.h"
#include "test.h"

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#define RRB_COUNT 2600
#define PREDEF_RRBS 200
#define MAX_INIT_SIZE (MIN(RRB_BRANCHING,16))

static const RRB* rand_rrb() {
  const uint32_t size = (uint32_t) (rand() % MAX_INIT_SIZE);
  const RRB *rrb = rrb_create();
  for (uint32_t i = 0; i < size; i++) {
    rrb = rrb_push(rrb, (void *) ((intptr_t) rand() & 0xffff));
  }
  return rrb;
}

int main(int argc, char *argv[]) {
  GC_INIT();
  setup_rand(argc == 2 ? argv[1] : NULL);

  int fail = 0;

  const RRB *rrbs[RRB_COUNT];

  for (uint32_t i = 0; i < PREDEF_RRBS; i++) {
    rrbs[i] = rand_rrb();
  }

  for (uint32_t i = PREDEF_RRBS; i < RRB_COUNT; i++) {
    rrbs[i] = rrb_concat(rrbs[i - PREDEF_RRBS], rrbs[i - PREDEF_RRBS + 1]);
  }

  for (uint32_t i = PREDEF_RRBS; i < RRB_COUNT; i++) {
    const RRB *merged = rrbs[i];
    const RRB *left = rrbs[i - PREDEF_RRBS];
    const RRB *right = rrbs[i - PREDEF_RRBS + 1];
    uint32_t merged_idx = 0;
    for (uint32_t left_idx = 0; left_idx < rrb_count(left); left_idx++, merged_idx++) {
      intptr_t expected = (intptr_t) rrb_nth(left, left_idx);
      intptr_t actual = (intptr_t) rrb_nth(merged, merged_idx);
      if (expected != actual) {
          printf("On multicatted RRB #%u, at merged element %u:\n",
                 i, merged_idx);
          printf("  Expected val at pos %u in left, %u in merged, to be %ld, but was %ld\n",
                 left_idx, merged_idx, expected, actual);
          fail = 1;
      }
    }
    for (uint32_t right_idx = 0; right_idx < rrb_count(right); right_idx++, merged_idx++) {
      intptr_t expected = (intptr_t) rrb_nth(right, right_idx);
      intptr_t actual = (intptr_t) rrb_nth(merged, merged_idx);
      if (expected != actual) {
          printf("On multicatted RRB #%u, at merged element %u:\n",
                 i, merged_idx);
          printf("  Expected val at pos %u in right, %u in merged, to be %ld, but was %ld\n",
                 right_idx, merged_idx, expected, actual);
          fail = 1;
      }
    }
  }

  return fail;
}
