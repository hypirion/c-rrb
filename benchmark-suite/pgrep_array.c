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

typedef struct {
  uint32_t from, to;
} Interval;

typedef struct {
  uint32_t len, cap;
  Interval *arr;
} IntervalArray;

IntervalArray* interval_array_create(void);
void interval_array_destroy(IntervalArray *int_arr);
void interval_array_add(IntervalArray *int_arr, Interval data);
void interval_array_concat(IntervalArray *left, IntervalArray *right);

IntervalArray* interval_array_create() {
  IntervalArray *container = malloc(sizeof(IntervalArray));
  Interval *arr = malloc(32 * sizeof(Interval));
  container->len = 0;
  container->cap = 32;
  container->arr = arr;
  return container;
}

void interval_array_destroy(IntervalArray *int_arr) {
  free(int_arr->arr);
  free(int_arr);
}

void interval_array_add(IntervalArray *int_arr, Interval data) {
  if (int_arr->len == int_arr->cap) {
    int_arr->cap *= 2;
    int_arr->arr = realloc(int_arr->arr, int_arr->cap * sizeof(Interval));
  }
  int_arr->arr[int_arr->len] = data;
  int_arr->len++;
}

void interval_array_concat(IntervalArray *left, IntervalArray *right) {
  if (left->cap < left->len + right->len) {
    left->cap = left->len + right->len;
    left->arr = realloc(left->arr, left->cap * sizeof(Interval));
  }
  memcpy(&left->arr[left->len], &right->arr[0], right->len * sizeof(Interval));
  left->len = left->len + right->len;
}

int main(int argc, char *argv[]) {
  // CLI argument parsing
  if (argc != 2) {
    fprintf(stderr, "Expected 1 argument, got %d\nExiting...\n", argc - 1);
    exit(1);
  }
  else {
    fprintf(stderr, "Your input was %s\n", argv[1]);

    FILE *fp;
    long file_size;
    char *buffer;
    fp = fopen(argv[1], "rb");
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

    IntervalArray *intervals = interval_array_create();
    uint32_t line_start = 0;
    for (uint32_t i = 0; i < file_size; i++) {
      if (buffer[i] == '\n') {
        Interval interval = {.from = line_start, .to = i};
        interval_array_add(intervals, interval);
        line_start = i + 1;
      }
    }

    fprintf(stdout, "%d newlines\n", intervals->len);

    free(buffer);
    exit(0);
  }
  
}
