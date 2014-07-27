/*
 * Copyright (c) 2014 Jean Niklas L'orange. All rights reserved.
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

#define SIZE 400000

int main() {
  GC_INIT();
  randomize_rand();

  int fail = 0;
  intptr_t *list = GC_MALLOC_ATOMIC(sizeof(intptr_t) * SIZE);
  for (uint32_t i = 0; i < SIZE; i++) {
    list[i] = (intptr_t) rand();
  }
  TransientRRB *trrb = rrb_to_transient(rrb_create());
  for (uint32_t i = 0; i < SIZE; i++) {
    trrb = transient_rrb_push(trrb, (void *) list[i]);
  }
  const RRB* rrb = transient_to_rrb(trrb);
  
  trrb = rrb_to_transient(rrb);
  for (uint32_t i = SIZE; 0 < i; i--) {
    intptr_t val = (intptr_t) transient_rrb_peek(trrb);
    if (val != list[i-1]) {
      printf("Expected val at pos %d to be %ld, was %ld.\n", i-i, list[i-1], val);
      fail = 1;
    }
    trrb = transient_rrb_pop(trrb);
  }

  return fail;
}
