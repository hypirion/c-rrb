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

#define TESTS 400
#define SIZE 40
#define MAX_EXTRA_PUSHES 50

/**
 * This test attempts to push values onto transient vectors, which originally
 * has been sliced or concatenated, or both. Mainly used to check that pushing
 * onto the vector handles size tables correctly, and that mutated pushes
 * doesn't flood into persistent RRB-trees.
 */
int main(int argc, char *argv[]) {
  GC_INIT();
  setup_rand(argc == 2 ? argv[1] : NULL);

  int fail = 0;
#ifdef TRANSIENTS
  for (uint32_t t = 0; t < TESTS; t++) {
    uint32_t extra_pushes = 32 + rand() % MAX_EXTRA_PUSHES;
    uint32_t rrb_size = SIZE % rand();
    
    intptr_t *list = GC_MALLOC_ATOMIC(sizeof(intptr_t) * rrb_size);
    intptr_t *extra_vals = GC_MALLOC_ATOMIC(sizeof(intptr_t) * extra_pushes);

    for (uint32_t i = 0; i < rrb_size; i++) {
      list[i] = (intptr_t) rand() % 0x1000;
    }
    for (uint32_t i = 0; i < extra_pushes; i++) {
      extra_vals[i] = (intptr_t) rand() % 0x1000;
    }

    // (single) concat check
    uint32_t rand_cut = rand() % rrb_size;
    const RRB *left = rrb_create(), *right = rrb_create();
    
    TransientRRB *tmp = rrb_to_transient(left);
    for (uint32_t i = 0; i < rand_cut; i++) {
      transient_rrb_push(tmp, (void *) list[i]);
    }
    left = transient_to_rrb(tmp);

    tmp = rrb_to_transient(right);
    for (uint32_t i = 0; i < rrb_size - rand_cut; i++) {
      transient_rrb_push(tmp, (void *) list[i + rand_cut]);
    }
    right = transient_to_rrb(tmp);

    const RRB* cat = rrb_concat(left, right);

    TransientRRB *trrb = rrb_to_transient(cat);
    for (uint32_t i = 0; i < extra_pushes; i++) {
      trrb = transient_rrb_push(trrb, (void *) extra_vals[i]);
    }
    
    // first check size
    if (transient_rrb_count(trrb) != extra_pushes + rrb_size) {
      printf("Expected size of transient rrb to be %u, but was %u.\n",
             extra_pushes + rrb_size, transient_rrb_count(trrb));
      fail = 1;
    }
    
    // then check that existant values are ok
    for (uint32_t i = 0; i < rrb_size; i++) {
      intptr_t actual = (intptr_t) transient_rrb_nth(trrb, i);
      intptr_t in_cat = (intptr_t) rrb_nth(cat, i);
      intptr_t expected = list[i];
      if (actual != expected) {
        printf("In run %u (catting):\n", t);
        printf("  Expected value at position %u to be %ld, but was %ld.\n",
               i, expected, actual);
        fail = 1;
      }
      if (actual != in_cat) {
        printf("In run %u (catting):\n", t);
        printf("  Value at position %u is %ld, differs from cat, which has %ld.\n",
               i, expected, in_cat);
        fail = 1;
      }
    }

    // then check last ones
    for (uint32_t i = 0; i < extra_pushes; i++) {
      intptr_t actual = (intptr_t) transient_rrb_nth(trrb, i + rrb_size);
      intptr_t expected = extra_vals[i];
      if (actual != expected) {
        printf("In run %u (catting):\n", t);
        printf("  Expected value at position %u to be %ld, but was %ld.\n",
               i + rrb_size, expected, actual);
        puts("  (Note: That's a pushed value)");
        fail = 1;
      }
    }

    // Finally, check the persistent result is ok
    const RRB* cat_pushed = transient_to_rrb(trrb);
    fail |= CHECK_TREE(cat_pushed);

    // slice check
    uint32_t left_offset, right_offset;
    left_offset = (uint32_t) rand() % rrb_size;
    right_offset = (uint32_t) (rand() % (rrb_size - left_offset)) + left_offset;

    const RRB* slice = rrb_slice(cat, left_offset, right_offset);

    trrb = rrb_to_transient(slice);
    for (uint32_t i = 0; i < extra_pushes; i++) {
      trrb = transient_rrb_push(trrb, (void *) extra_vals[i]);
    }

    // check that sliced ones are correctly inserted
    for (uint32_t i = 0; i  < rrb_count(slice); i++) {
      intptr_t actual = (intptr_t) transient_rrb_nth(trrb, i);
      intptr_t in_slice = (intptr_t) rrb_nth(slice, i);
      intptr_t expected = list[i + left_offset];
      if (actual != expected) {
        printf("In run %u (slicing):\n", t);
        printf("  Expected value at position %u to be %ld, but was %ld.\n",
               i, expected, actual);
        fail = 1;
      }
      if (actual != in_slice) {
        printf("In run %u (slicing):\n", t);
        printf("  Value at position %u is %ld, differs from slice, which has %ld.\n",
               i, expected, in_slice);
        fail = 1;
      }
    }

    // check that pushed ones are correctly inserted
    for (uint32_t i = 0; i < extra_pushes; i++) {
      intptr_t actual = (intptr_t) transient_rrb_nth(trrb, i + rrb_count(slice));
      intptr_t expected = extra_vals[i];
      if (actual != expected) {
        printf("In run %u (sliced):\n", t);
        printf("  Expected value at position %u to be %ld, but was %ld.\n",
               i + rrb_count(slice), expected, actual);
        puts("  (Note: That's a pushed value)");
        fail = 1;
      }
    }    

    // Again, we end up checking that the persistent result is ok
    const RRB* slice_pushed = transient_to_rrb(trrb);
    fail |= CHECK_TREE(slice_pushed);
  }
#endif
  return fail;
}
