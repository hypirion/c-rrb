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

#ifdef TRANSIENTS

#include "rrb_thread.h"

struct _TransientRRB {
  uint32_t cnt;
  uint32_t shift;
#ifdef RRB_TAIL
  uint32_t tail_len;
  LeafNode *tail;
#endif
  TreeNode *root;
  RRBThread owner;
  GUID_DECLARATION
};


static void* rrb_guid_create(void);
static TransientRRB* transient_rrb_head_create(const RRB* rrb);
static void check_transience(const TransientRRB *trrb);
static InternalNode* ensure_internal_editable(InternalNode *internal, void *guid);
static LeafNode* ensure_leaf_editable(LeafNode *leaf, void *guid);

static void* rrb_guid_create() {
  return (void *) RRB_MALLOC_ATOMIC(1);
}

// creates a new guid -- at least for now. Check if necessary later on.
static TransientRRB* transient_rrb_head_create(const RRB* rrb) {
  TransientRRB *trrb = RRB_MALLOC(sizeof(TransientRRB));
  memcpy(rrb, trrb, sizeof(RRB));
  trrb->owner = RRB_THREAD_ID();
  trrb->guid = rrb_guid_create();
  return trrb;
}

static void check_transience(const TransientRRB *trrb) {
  if (trrb->guid == NULL) {
    // Transient used after transient_to_persistent call
    exit(1);
  }
  if (!RRB_THREAD_EQUALS(trrb->owner, RRB_THREAD_ID())) {
    // Transient used by non-owner thread
    exit(1);
  }
}

static InternalNode* ensure_internal_editable(InternalNode *internal, void *guid) {
  if (internal->guid == guid) {
    return internal;
  }
  else {
    InternalNode *copy = internal_node_clone(internal);
    copy->guid = guid;
    return copy;
  }
}

static LeafNode* ensure_leaf_editable(LeafNode *leaf, void *guid) {
  if (leaf->guid == guid) {
    return leaf;
  }
  else {
    LeafNode *copy = leaf_node_clone(leaf);
    copy->guid = guid;
    return copy;
  }
}


#endif
