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

static InternalNode* transient_internal_node_create(void);
static LeafNode* transient_leaf_node_create(void);
static InternalNode* transient_internal_node_clone(const InternalNode *internal, void *guid);
static LeafNode* transient_leaf_node_clone(const LeafNode *leaf, void *guid);

static InternalNode* ensure_internal_editable(InternalNode *internal, void *guid);
static LeafNode* ensure_leaf_editable(LeafNode *leaf, void *guid);

static void* rrb_guid_create() {
  return (void *) RRB_MALLOC_ATOMIC(1);
}

static TransientRRB* transient_rrb_head_create(const RRB* rrb) {
  TransientRRB *trrb = RRB_MALLOC(sizeof(TransientRRB));
  memcpy(rrb, trrb, sizeof(RRB));
  trrb->owner = RRB_THREAD_ID();
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

static InternalNode* transient_internal_node_create() {
  InternalNode *node = RRB_MALLOC(sizeof(InternalNode)
                              + RRB_BRANCHING * sizeof(InternalNode *));
  node->type = INTERNAL_NODE;
  node->size_table = NULL;
  return node;
}

static LeafNode* transient_leaf_node_create() {
  LeafNode *node = RRB_MALLOC(sizeof(LeafNode)
                                  + RRB_BRANCHING * sizeof(void *));
  node->type = LEAF_NODE;
  return node;
}

static InternalNode* transient_internal_node_clone(const InternalNode *internal, void *guid) {
  InternalNode *copy = transient_internal_node_create();
  memcpy(copy, internal,
         sizeof(InternalNode) + internal->len * sizeof(InternalNode *));
  copy->guid = guid;
  return copy;
}

static LeafNode* transient_leaf_node_clone(const LeafNode *leaf, void *guid) {
  LeafNode *copy = transient_leaf_node_create();
  memcpy(copy, leaf, sizeof(LeafNode) + leaf->len * sizeof(void *));
  copy->guid = guid;
  return copy;
}

static InternalNode* ensure_internal_editable(InternalNode *internal, void *guid) {
  if (internal->guid == guid) {
    return internal;
  }
  else {
    return transient_internal_node_clone(internal, guid);
  }
}

static LeafNode* ensure_leaf_editable(LeafNode *leaf, void *guid) {
  if (leaf->guid == guid) {
    return leaf;
  }
  else {
    return transient_leaf_node_clone(leaf, guid);
  }
}

TransientRRB* rrb_to_transient(const RRB *rrb) {
  TransientRRB* trrb = transient_rrb_head_create(rrb);
  const void *guid = rrb_guid_create();
  trrb->guid = guid;
#ifdef RRB_TAIL
  trrb->tail = transient_leaf_node_clone(rrb->tail, guid);
#endif
  if (trrb->root != NULL) {
    if (trrb->shift == LEAF_NODE_SHIFT) {
      trrb->root = transient_leaf_node_clone((const LeafNode *)rrb->root, guid);
    }
    else {
      trrb->root = transient_internal_node_clone(rrb->root, guid);
    }
  }
  return trrb;
}

void* transient_rrb_nth(const TransientRRB *trrb, uint32_t index) {
  return rrb_nth((const RRB *) trrb, index);
}

void* transient_rrb_peek(const TransientRRB *trrb) {
  return transient_rrb_nth(trrb, trrb->cnt - 1);
}

#endif
