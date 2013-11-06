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

#define SIZE 10000

int main() {
  GC_INIT();

  time_t timestamp = time(NULL);
  printf("Timestamp for this run: %llu\n", (unsigned long long) timestamp);
  srand((unsigned int) timestamp);

  int fail = 0;
  
  const RRB *rrb1 = rrb_create();
  const RRB *rrb2 = rrb_create();
  for (uint32_t i = 0; i < SIZE; i++) {
    rrb1 = rrb_push(rrb1, (intptr_t) rand());
    rrb2 = rrb_push(rrb2, (intptr_t) rand());
    
    const RRB* catted = rrb_concat(rrb1, rrb2);
    for (uint32_t j = 0; j < (i + 1) * 2; j++) {
      intptr_t val_cat = (intptr_t) rrb_nth(catted, j);
      if (j <= i) {
        intptr_t val1 = (intptr_t) rrb_nth(rrb1, j);
        if (val1 != val_cat) {
          printf("Expected val at pos %d to be %ld (left rrb), was %ld.\n",
                 i, val1, val_cat);
          fail = 1;
        }
      }
      else { // if (j > i)
        intptr_t val2 = (intptr_t) rrb_nth(rrb2, j - i - 1);
        if (val2 != val_cat) {
          printf("Expected val at pos %d to be %ld (right rrb), was %ld.\n",
                 i, val2, val_cat);
          fail = 1;
        }
      }
    }
  }
  return fail;
}
