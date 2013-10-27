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

#ifndef RRB_H
#define RRB_H

#include <stdint.h>

#define RRB_BITS 2
#define RRB_BRANCHING (1 << RRB_BITS)
#define RRB_MASK (RRB_BRANCHING - 1)
#define RRB_INVARIANT 1
#define RRB_EXTRAS 0

typedef struct _RRB RRB;

const RRB* rrb_create(void);
void rrb_destroy(RRB *restrict rrb);

uint32_t rrb_count(const RRB *restrict rrb);
void* rrb_nth(const RRB *restrict rrb, const uint32_t pos);
const RRB* rrb_pop(const RRB *restrict rrb);
void* rrb_peek(const RRB *restrict rrb);
const RRB* rrb_push(const RRB *restrict rrb, const void *restrict elt);
const RRB* rrb_update(const RRB *restrict rrb, uint32_t pos, void *restrict elt);

// TODO: Is it okay to use restrict here?
const RRB* rrb_concat(const RRB *first, const RRB *second);
const RRB* rrb_slice(const RRB *restrict rrb, uint32_t from, uint32_t to);

#ifdef RRB_DEBUG

void rrb_to_dot_file(const RRB *rrb, char *loch);

#endif
#endif
