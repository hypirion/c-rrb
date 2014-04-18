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

#define SIZE 400
#define SLICED 53
#define CATTED 2310
#define TOT_CATTED 10

int main(int argc, char *argv[]) {
  GC_INIT();
  setup_rand(argc == 2 ? argv[1] : NULL);

  int fail = 0;
  
  const RRB *rrb = rrb_create();
  for (uint32_t i = 0; i < SIZE; i++) {
    rrb = rrb_push(rrb, (void *)((intptr_t) rand() % 10000));
  }

  const RRB **sliced = GC_MALLOC(sizeof(RRB *) * SLICED);
  for (uint32_t i = 0; i < SLICED; i++) {
    uint32_t from = (uint32_t) rand() % SIZE;
    uint32_t to = (uint32_t) (rand() % (SIZE - from)) + from;
    sliced[i] = rrb_slice(rrb, from, to);
  }

  for (uint32_t i = 0; i < CATTED; i++) {
    const RRB *multicat = rrb_create(); // Originally empty
    uint32_t tot_cats = (uint32_t) rand() % TOT_CATTED;
    uint32_t *merged_in = GC_MALLOC_ATOMIC(sizeof(uint32_t) * tot_cats);
    for (uint32_t cat_num = 0; cat_num < tot_cats; cat_num++) {
      merged_in[cat_num] = (uint32_t) rand() % SLICED;
      multicat = rrb_concat(multicat, sliced[merged_in[cat_num]]);
    }
    
    // checking consistency here
    uint32_t pos = 0;
    uint32_t merged_pos = 0;
    while (pos < rrb_count(multicat)) {
      const RRB *merged = sliced[merged_in[merged_pos]];
      
      for (uint32_t merged_i = 0; merged_i < rrb_count(merged);
           merged_i++, pos++) {
        intptr_t expected = (intptr_t) rrb_nth(merged, merged_i);
        intptr_t actual = (intptr_t) rrb_nth(multicat, pos);
        if (expected != actual) {
          printf("On multicatted object #%u, at merged element %u:\n",
                 i, merged_pos);
          printf("  Expected val at pos %u (%u in merged) to be %ld, but was %ld\n",
                 pos, merged_i, expected, actual);
          printf("  Size of merged: %u, is at index %u\n", rrb_count(merged),
                 merged_in[merged_pos]);
          printf("Sliced lists:\n");
          fail = 1;
          return fail;
        }
      }
      merged_pos++;
    }
  }
  return fail;
}
