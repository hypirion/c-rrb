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

#ifndef SUBSTR_CONTAINS_H
#define SUBSTR_CONTAINS_H

char substr_contains(const char *str, const uint32_t len, const char *target);

char substr_contains(const char *str, const uint32_t len, const char *target) {
  uint32_t start_idx = 0;
  while (start_idx < len) {
    uint32_t idx = start_idx;
    char *t_char = (char *) target;
    while (idx < len && *t_char && str[idx] == *t_char) {
      idx++;
      t_char++;
    }
    if (!*t_char)
      return 1;
    start_idx++;
  }
  return 0;
}
#endif
