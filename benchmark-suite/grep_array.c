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
#include <stdint.h>
#include <string.h>
#include "interval.h"
#include "substr_contains.h"

int main(int argc, char *argv[]) {
  // CLI argument parsing
  if (argc != 3) {
    fprintf(stderr, "Expected 2 arguments, got %d\nExiting...\n", argc - 1);
    exit(1);
  }
  else {
    fprintf(stderr, "Looking for '%s' in file %s...\n", argv[1], argv[2]);

    char *search_term = argv[1];

    FILE *fp;
    long file_size;
    char *buffer;
    fp = fopen(argv[2], "rb");
    if (!fp) {
      perror(argv[1]);
      exit(1);
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    buffer = malloc(file_size + 1);
    if (!buffer) {
      fclose(fp);
      fprintf(stderr, "Cannot allocate buffer for file (probably too large).\n");
      exit(1);
    }
    buffer[file_size] = 0;

    if (1 != fread(buffer, file_size, 1, fp)) {
      fclose(fp);
      free(buffer);
      fprintf(stderr, "Entire read failed.\n");
      exit(1);
    }
    else {
      fclose(fp);
    }

    // Find all lines
    IntervalArray *intervals = interval_array_create();
    uint32_t line_start = 0;
    for (uint32_t i = 0; i < file_size; i++) {
      if (buffer[i] == '\n') {
        Interval interval = {.from = line_start, .to = i};
        interval_array_add(intervals, interval);
        line_start = i + 1;
      }
    }

    fprintf(stderr, "%d newlines\n", intervals->len);

    // Find all lines containing search term
    IntervalArray *contained_intervals = interval_array_create();

    for (uint32_t line_idx = 0; line_idx < intervals->len; line_idx++) {
      Interval line = interval_array_nth(intervals, line_idx);
      if (substr_contains(&buffer[line.from], line.to - line.from, search_term)) {
        interval_array_add(contained_intervals, line);
      }
    }


    fprintf(stderr, "%d hits\n", contained_intervals->len);

    interval_array_destroy(intervals);
    interval_array_destroy(contained_intervals);
    free(buffer);
    exit(0);
  }

}
