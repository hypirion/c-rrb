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

#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "rrb.h"

int main() {
  uint32_t top = 40;
  const RRB *rrb1 = rrb_create();
  const RRB *rrb2 = rrb_create();
  for (uint32_t i = 0; i < top; i++) {
    rrb1 = rrb_push(rrb1, (uintptr_t) i);
    rrb2 = rrb_push(rrb2, (uintptr_t) (i + top));
    char str[80];
    sprintf(str, "img/test-%03d.dot", i);
    const RRB* catted = rrb_concat(rrb1, rrb2);
    rrb_to_dot_file(catted, str);
    printf("img/test-%03d.dot\n", i);
    for (uint32_t j = 0; j < rrb_count(catted); j++) {
      printf("%lx ", (uintptr_t) rrb_nth(catted, j));
    }
    puts("\n\n");
    if (i > 10) {
      const RRB* sliced = rrb_slice(catted, 5, (uint32_t) (i - 5));
      sprintf(str, "img/sliced-%03d.dot", i);
      rrb_to_dot_file(sliced, str);
      printf("img/sliced-%03d.dot\n", i);
      for (uint32_t j = 0; j < rrb_count(sliced); j++) {
        printf("%lx ", (uintptr_t) rrb_nth(sliced, j));
      }
      puts("\n\n");
    }
    // testing updates here
    const RRB* updated = rrb_update(catted, i, (uintptr_t) 0x1337);
    sprintf(str, "img/updated-%03d.dot", i);
    DotFile dot = dot_file_create(str);
    rrb_to_dot(dot, updated);
    label_pointer(dot, updated, "updated");
    rrb_to_dot(dot, catted);
    label_pointer(dot, catted, "catted");
    dot_file_close(dot);
    printf("img/updated-%03d.dot\n", i);
    for (uint32_t j = 0; j < rrb_count(updated); j++) {
      printf("%lx ", (uintptr_t) rrb_nth(updated, j));
    }
    puts("\n\n");
  }
}
