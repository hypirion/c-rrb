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

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "interval.h"

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

Interval interval_array_nth(IntervalArray *int_arr, uint32_t index) {
  if (index >= int_arr->len) {
    Interval empty = {0, 0};
    return empty;
  }
  else {
    return int_arr->arr[index];
  }
}

void interval_array_concat(IntervalArray *left, IntervalArray *right) {
  if (left->cap < left->len + right->len) {
    left->cap = left->len + right->len;
    left->arr = realloc(left->arr, left->cap * sizeof(Interval));
  }
  memcpy(&left->arr[left->len], &right->arr[0], right->len * sizeof(Interval));
  left->len = left->len + right->len;
}
