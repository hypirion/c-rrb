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

#ifndef RRB_H
#define RRB_H

#include <stdint.h>

#ifndef HAVE_CONFIG_H
#warning "You don't seem to use config.h -- most efficient options are picked."
#define DIRECT_APPEND 1
#else
#include <config.h>
#endif

#ifndef RRB_BITS
#define RRB_BITS 5
#define RRB_MAX_HEIGHT 7
#endif
#define RRB_BRANCHING (1 << RRB_BITS)
#define RRB_MASK (RRB_BRANCHING - 1)
#ifndef RRB_INVARIANT
#define RRB_INVARIANT 1
#endif
#ifndef RRB_EXTRAS
#define RRB_EXTRAS 2
#endif

typedef struct _RRB RRB;

const RRB* rrb_create(void);

uint32_t rrb_count(const RRB *rrb);
void* rrb_nth(const RRB *rrb, uint32_t index);
const RRB* rrb_pop(const RRB *rrb);
void* rrb_peek(const RRB *rrb);
const RRB* rrb_push(const RRB *restrict rrb, const void *restrict elt);
const RRB* rrb_update(const RRB *restrict rrb, uint32_t index, const void *restrict elt);

const RRB* rrb_concat(const RRB *left, const RRB *right);
const RRB* rrb_slice(const RRB *rrb, uint32_t from, uint32_t to);

#ifdef RRB_DEBUG

#include <stdio.h>

typedef struct _DotFile DotFile;

void rrb_to_dot_file(const RRB *rrb, char *loch);

DotFile dot_file_create(char *loch);
void dot_file_close(DotFile dot);

void label_pointer(DotFile dot, const void *node, const char *name);
void rrb_to_dot(DotFile dot, const RRB *rrb);

uint32_t rrb_memory_usage(const RRB *const *rrbs, uint32_t rrb_count);

#endif
#endif
