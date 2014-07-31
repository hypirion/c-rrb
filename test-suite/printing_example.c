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

// It's more interesting to try out branching factors by using
//   ./configure --with-branching=2
// instead of the default 5. Way easier to see what's going on!

int main() {
  GC_INIT();
  const RRB *rrb = rrb_create();

  for (uintptr_t i = 0; i < 33; i++) {
    rrb = rrb_push(rrb, (void *) i);
  }
  int file_size = rrb_to_dot_file(rrb, "foo.dot");
  switch (file_size) {
  case -1:
    puts("Had trouble either opening or closing \"foo.dot\".");
    return 1;
  case -2:
    puts("Had trouble writing to the file \"foo.dot\".");
    return 1;
  default:
    printf("The file \"foo.dot\" contains an RRB-tree in dot format "
           "(%d bytes)\n", file_size);
    if (RRB_BITS != 2) {
      puts("\n"
"(If you want to use the code for visualising RRB-trees, I'd recommend to use the\n"
" settings `CFLAGS='-Ofast' ./configure --with-branching=2` instead. It is much\n"
" easier to visualise/comprehend with 4 elements per trie node instead of 32.)");
    }
    return 0;
  }
}
