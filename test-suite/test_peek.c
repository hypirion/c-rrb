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

#define SIZE 400000

int main() {
  GC_INIT();
  randomize_rand();

  int fail = 0;
  intptr_t *list = GC_MALLOC_ATOMIC(sizeof(intptr_t) * SIZE);
  for (uint32_t i = 0; i < SIZE; i++) {
    list[i] = (intptr_t) rand();
  }
  const RRB *rrb = rrb_create();
  for (uint32_t i = 0; i < SIZE; i++) {
    rrb = rrb_push(rrb, (void *) list[i]);
    intptr_t val = (intptr_t) rrb_peek(rrb);
    if (val != list[i]) {
      printf("Expected val at pos %d to be %ld, was %ld.\n", i, list[i], val);
      fail = 1;
    }
  }
  return fail;
}
