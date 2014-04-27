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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "rrb.h"

void randomize_rand(void);
void print_rrb(const RRB *rrb);
void setup_rand(const char *str_seed);

#ifdef RRB_DEBUG
#define CHECK_TREE(t) (validate_rrb(t))
#else
#define CHECK_TREE(t) (0)
#endif

void setup_rand(const char *str_seed) {
  if (str_seed == NULL) {
    randomize_rand();
  } else {
    unsigned int seed = (unsigned int) atol(str_seed);
    printf("Seed for this run: %u\n", seed);
    fflush(stdout);
    srand(seed);
  }
}

void randomize_rand() {
  time_t timestamp = time(NULL);
  printf("Seed for this run: %u\n", (unsigned int) timestamp);
  srand((unsigned int) timestamp);
}

void print_rrb(const RRB *rrb) {
  uint32_t count = rrb_count(rrb);
  printf("[");
  char sep = 0;
  for (uint32_t i = 0; i < count; i++) {
    intptr_t val = (intptr_t) rrb_nth(rrb, i);
    printf("%s%ld", sep ? ", " : "", val);
    sep = 1;
  }
  printf("]\n");
}
