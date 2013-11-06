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

#include <gc/gc.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "rrb.h"

#define SIZE   40000
#define SLICES 10000

int main() {
  GC_INIT();

  time_t timestamp = time(NULL);
  printf("Timestamp for this run: %llu\n", (unsigned long long) timestamp);
  srand((unsigned int) timestamp);

  int fail = 0;
  intptr_t *list = GC_MALLOC_ATOMIC(sizeof(intptr_t) * SIZE);
  for (uint32_t i = 0; i < SIZE; i++) {
    list[i] = (intptr_t) rand();
  }
  
  const RRB *rrb = rrb_create();
  for (uint32_t i = 0; i < SIZE; i++) {
    rrb = rrb_push(rrb, (void *) list[i]);
  }

  uint32_t *from_list = GC_MALLOC_ATOMIC(sizeof(int) * SLICES);
  uint32_t *to_list = GC_MALLOC_ATOMIC(sizeof(int) * SLICES);
  for (uint32_t i = 0; i < SLICES; i++) {
    from_list[i] = (uint32_t) rand() % SIZE;
    to_list[i] = (uint32_t) (rand() % (SIZE - from_list[i])) + from_list[i];
  }

  for (uint32_t i = 0; i < SLICES; i++) {
    const RRB *sliced = rrb_slice(rrb, from_list[i], to_list[i]);
    for (uint32_t j = 0; j < rrb_count(sliced); j++) {
      intptr_t sliced_val = (intptr_t) rrb_nth(sliced, j);
      intptr_t original_val = (intptr_t) rrb_nth(rrb, j + from_list[i]);
      if (sliced_val != original_val) {
        printf("On iteration %u (bounds [%u, %u]):\n", i, from_list[i],
               to_list[i]);
        printf("  Expected val at pos %u to be %ld, was %ld.\n", j,
               original_val, sliced_val);
        fail = 1;
      }
    }
  }
  
  return fail;
}
