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

#include "rrb_alloc.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "rrb.h"


#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

// Typical stuff
#define RRB_SHIFT(rrb) (rrb->shift)
#define INC_SHIFT(shift) (shift + (uint32_t) RRB_BITS)
#define DEC_SHIFT(shift) (shift - (uint32_t) RRB_BITS)
#define LEAF_NODE_SHIFT ((uint32_t) 0)

#ifdef TRANSIENTS
// Abusing allocated pointers being unique to create GUIDs: using a single
// malloc to create a guid.
#define GUID_DECLARATION const void *guid;
#else
#define GUID_DECLARATION
#endif

typedef enum {LEAF_NODE, INTERNAL_NODE} NodeType;

typedef struct TreeNode {
  NodeType type;
  uint32_t len;
  GUID_DECLARATION
} TreeNode;

typedef struct LeafNode {
  NodeType type;
  uint32_t len;
  GUID_DECLARATION
  const void *child[];
} LeafNode;

typedef struct RRBSizeTable {
  char t; // Dummy variable to avoid empty struct. FIXME
  GUID_DECLARATION
  uint32_t size[];
} RRBSizeTable;

typedef struct InternalNode {
  NodeType type;
  uint32_t len;
  GUID_DECLARATION
  RRBSizeTable *size_table;
  struct InternalNode *child[];
} InternalNode;

struct _RRB {
  uint32_t cnt;
  uint32_t shift;
#ifdef RRB_TAIL
  uint32_t tail_len;
  LeafNode *tail;
#endif
  TreeNode *root;
};

// perhaps point to an empty leaf to remove edge cases?
#ifdef RRB_TAIL
static LeafNode EMPTY_LEAF = {.type = LEAF_NODE, .len = 0};
static const RRB EMPTY_RRB = {.cnt = 0, .shift = 0, .root = NULL,
                              .tail_len = 0, .tail = &EMPTY_LEAF};
#else
static const RRB EMPTY_RRB = {.cnt = 0, .shift = 0, .root = NULL};
#endif

static RRBSizeTable* size_table_create(uint32_t len);
static RRBSizeTable* size_table_clone(const RRBSizeTable* original, uint32_t len);
static RRBSizeTable* size_table_inc(const RRBSizeTable *original, uint32_t len);

static InternalNode* concat_sub_tree(TreeNode *left_node, uint32_t left_shift,
                                     TreeNode *right_node, uint32_t right_shift,
                                     char is_top);
static InternalNode* rebalance(InternalNode *left, InternalNode *centre,
                               InternalNode *right, uint32_t shift,
                               char is_top);
static uint32_t* create_concat_plan(InternalNode *all, uint32_t *top_len);
static InternalNode* execute_concat_plan(InternalNode *all, uint32_t *node_sizes,
                                         uint32_t slen, uint32_t shift);
static uint32_t find_shift(TreeNode *node);
static InternalNode* set_sizes(InternalNode *node, uint32_t shift);
static uint32_t size_sub_trie(TreeNode *node, uint32_t parent_shift);
static uint32_t sized_pos(const InternalNode *node, uint32_t *index,
                          uint32_t sp);
static const InternalNode* sized(const InternalNode *node, uint32_t *index,
                                 uint32_t sp);

static LeafNode* leaf_node_clone(const LeafNode *original);
static LeafNode* leaf_node_inc(const LeafNode *original);
static LeafNode* leaf_node_dec(const LeafNode *original);
static LeafNode* leaf_node_create(uint32_t size);
static LeafNode* leaf_node_merge(LeafNode *left_leaf, LeafNode *right_leaf);

static InternalNode* internal_node_create(uint32_t len);
static InternalNode* internal_node_clone(const InternalNode *original);
static InternalNode* internal_node_inc(const InternalNode *original);
static InternalNode* internal_node_dec(const InternalNode *original);
static InternalNode* internal_node_merge(InternalNode *left, InternalNode *centre,
                                         InternalNode *right);
static InternalNode* internal_node_copy(InternalNode *original, uint32_t start,
                                        uint32_t len);
static InternalNode* internal_node_new_above1(InternalNode *child);
static InternalNode* internal_node_new_above(InternalNode *left, InternalNode *right);

static RRB* slice_right(const RRB *rrb, const uint32_t right);
static TreeNode* slice_right_rec(uint32_t *total_shift, const TreeNode *root,
                                  uint32_t right, uint32_t shift,
                                  char has_left);
static const RRB* slice_left(RRB *rrb, uint32_t left);
static TreeNode* slice_left_rec(uint32_t *total_shift, const TreeNode *root,
                                uint32_t left, uint32_t shift,
                                char has_right);

static RRB* rrb_head_create(TreeNode *node, uint32_t size, uint32_t shift);
static RRB* rrb_head_clone(const RRB *original);

#ifdef RRB_TAIL
static RRB* push_down_tail(const RRB *restrict rrb, RRB *restrict new_rrb,
                           const LeafNode *restrict new_tail);
static void promote_rightmost_leaf(RRB *new_rrb);

#define IF_TAIL(a, b) a

#else
#define IF_TAIL(a, b) b
#endif

#ifdef RRB_DEBUG
#include <stdio.h>
#include <stdarg.h>

typedef struct _DotArray {
  uint32_t len;
  uint32_t cap;
  const void **elems;
} DotArray;

struct _DotFile {
  FILE *file;
  DotArray *array;
};

static DotArray* dot_array_create(void);
static char dot_array_contains(const DotArray *arr, const void *elem);
static void dot_array_add(DotArray *arr, const void *elem);

static void tree_node_to_dot(DotFile dot, const TreeNode *node, char print_table);
static void leaf_node_to_dot(DotFile dot, const LeafNode *root);
static void internal_node_to_dot(DotFile dot, const InternalNode *root, char print_table);
static void size_table_to_dot(DotFile dot, const InternalNode *node);

static uint32_t node_size(DotArray *arr, const TreeNode *node);
#endif

static RRBSizeTable* size_table_create(uint32_t size) {
  RRBSizeTable *table = RRB_MALLOC(sizeof(RRBSizeTable)
                                  + size * sizeof(uint32_t));
  return table;
}

static RRBSizeTable* size_table_clone(const RRBSizeTable *original,
                                      uint32_t len) {
  RRBSizeTable *clone = RRB_MALLOC(sizeof(RRBSizeTable)
                                   + len * sizeof(uint32_t));
  memcpy(&clone->size, &original->size, sizeof(uint32_t) * len);
  return clone;
}

static inline RRBSizeTable* size_table_inc(const RRBSizeTable *original,
                                           uint32_t len) {
  RRBSizeTable *incr = RRB_MALLOC(sizeof(RRBSizeTable) +
                                  (len + 1) * sizeof(uint32_t));
  memcpy(&incr->size, &original->size, sizeof(uint32_t) * len);
  return incr;
}

static RRB* rrb_head_create(TreeNode *node, uint32_t size, uint32_t shift) {
  RRB *rrb = RRB_MALLOC(sizeof(RRB));
  rrb->root = node;
  rrb->cnt = size;
  rrb->shift = shift;
  return rrb;
}

static RRB* rrb_head_clone(const RRB* original) {
  RRB *clone = RRB_MALLOC(sizeof(RRB));
  memcpy(clone, original, sizeof(RRB));
  return clone;
}

const RRB* rrb_create() {
  return &EMPTY_RRB;
}

static RRB* rrb_mutable_create() {
  RRB *rrb = RRB_MALLOC(sizeof(RRB));
  return rrb;
}

const RRB* rrb_concat(const RRB *left, const RRB *right) {
  if (left->cnt == 0) {
    return right;
  }
  else if (right->cnt == 0) {
    return left;
  }
  else {
#ifdef RRB_TAIL
    if (right->root == NULL) {
      // merge left and right tail, if possible
      RRB *new_rrb = rrb_head_clone(left);
      new_rrb->cnt += right->cnt;

      // skip merging if left tail is full.
      if (left->tail_len == RRB_BRANCHING) {
        new_rrb->tail_len = right->tail_len;
        return push_down_tail(left, new_rrb, right->tail);
      }
      // We can merge both tails into a single tail.
      else if (left->tail_len + right->tail_len <= RRB_BRANCHING) {
        const uint32_t new_tail_len = left->tail_len + right->tail_len;
        LeafNode *new_tail = leaf_node_merge(left->tail, right->tail);
        new_rrb->tail = new_tail;
        new_rrb->tail_len = new_tail_len;
        return new_rrb;
      }
      else { // must push down something, and will have elements remaining in
             // the right tail
        LeafNode *push_down = leaf_node_create(RRB_BRANCHING);
        memcpy(&push_down->child[0], &left->tail->child[0],
               left->tail_len * sizeof(void *));
        const uint32_t right_cut = RRB_BRANCHING - left->tail_len;
        memcpy(&push_down->child[left->tail_len], &right->tail->child[0],
               right_cut * sizeof(void *));

        // this will be strictly positive.
        const uint32_t new_tail_len = right->tail_len - right_cut;
        LeafNode *new_tail = leaf_node_create(new_tail_len);

        memcpy(&new_tail->child[0], &right->tail->child[right_cut],
               new_tail_len * sizeof(void *));

        new_rrb->tail = push_down;
        new_rrb->tail_len = new_tail_len;

        // This is an imitation, so that push_down_tail works as we intend it
        // to: Whenever the height has to be increased, it calculates the size
        // table based upon the old rrb's size, minus the old tail. However,
        // since we manipulate the old tail to be longer than it actually was,
        // we have to reflect those changes in the cnt variable.
        RRB left_imitation;
        memcpy(&left_imitation, left, sizeof(RRB));
        left_imitation.cnt = new_rrb->cnt - new_tail_len;

        return push_down_tail(&left_imitation, new_rrb, new_tail);
      }
    }
    left = push_down_tail(left, rrb_head_clone(left), NULL);
#endif
    RRB *new_rrb = rrb_mutable_create();
    new_rrb->cnt = left->cnt + right->cnt;

    InternalNode *root_candidate = concat_sub_tree(left->root, RRB_SHIFT(left),
                                                   right->root, RRB_SHIFT(right),
                                                   true);

#ifndef RRB_TAIL
    // Is there enough space in a leaf node? If so, pick that leaf node as root.
    if ((RRB_SHIFT(left) == LEAF_NODE_SHIFT) && (RRB_SHIFT(right) == LEAF_NODE_SHIFT)
        && ((left->cnt + right->cnt) <= RRB_BRANCHING)) {
      new_rrb->root = (TreeNode *) root_candidate->child[0];
      new_rrb->shift = find_shift(new_rrb->root);
    }
    else {
#endif
      new_rrb->shift = find_shift((TreeNode *) root_candidate);
      // must be done before we set sizes.
      new_rrb->root = (TreeNode *) set_sizes(root_candidate,
                                             RRB_SHIFT(new_rrb));
#ifndef RRB_TAIL
    }
#else
    new_rrb->tail = right->tail;
    new_rrb->tail_len = right->tail_len;
#endif
    return new_rrb;
  }
}

static InternalNode* concat_sub_tree(TreeNode *left_node, uint32_t left_shift,
                                     TreeNode *right_node, uint32_t right_shift,
                                     char is_top) {
  if (left_shift > right_shift) {
    // Left tree is higher than right tree
    InternalNode *left_internal = (InternalNode *) left_node;
    InternalNode *centre_node =
      concat_sub_tree((TreeNode *) left_internal->child[left_internal->len - 1],
                      DEC_SHIFT(left_shift),
                      right_node, right_shift,
                      false);
    return rebalance(left_internal, centre_node, NULL, left_shift, is_top);
  }
  else if (left_shift < right_shift) {
    InternalNode *right_internal = (InternalNode *) right_node;
    InternalNode *centre_node =
      concat_sub_tree(left_node, left_shift,
                      (TreeNode *) right_internal->child[0],
                      DEC_SHIFT(right_shift),
                      false);
    return rebalance(NULL, centre_node, right_internal, right_shift, is_top);
  }
  else { // we have same height
    if (left_shift == LEAF_NODE_SHIFT) { // We're dealing with leaf nodes
      LeafNode *left_leaf = (LeafNode *) left_node;
      LeafNode *right_leaf = (LeafNode *) right_node;
      // We don't do this if we're not at top, as we'd have to zip stuff above
      // as well.
      if (is_top && (left_leaf->len + right_leaf->len) <= RRB_BRANCHING) {
        // Can put them in a single node
        LeafNode *merged = leaf_node_merge(left_leaf, right_leaf);
        return internal_node_new_above1((InternalNode *) merged);
      }
      else {
        InternalNode *left_internal = (InternalNode *) left_node;
        InternalNode *right_internal = (InternalNode *) right_node;
        return internal_node_new_above(left_internal, right_internal);
      }
    }

    else { // two internal nodes with same height. Move both down
      InternalNode *left_internal = (InternalNode *) left_node;
      InternalNode *right_internal = (InternalNode *) right_node;
      InternalNode *centre_node =
        concat_sub_tree((TreeNode *) left_internal->child[left_internal->len - 1],
                        DEC_SHIFT(left_shift),
                        (TreeNode *) right_internal->child[0],
                        DEC_SHIFT(right_shift),
                        false);
      // can be optimised: since left_shift == right_shift, we'll end up in this
      // block again.
      return rebalance(left_internal, centre_node, right_internal, left_shift,
                       is_top);
    }
  }
}

static LeafNode* leaf_node_clone(const LeafNode *original) {
  size_t size = sizeof(LeafNode) + original->len * sizeof(void *);
  LeafNode *clone = RRB_MALLOC(size);
  memcpy(clone, original, size);
  return clone;
}

static LeafNode* leaf_node_inc(const LeafNode *original) {
  size_t size = sizeof(LeafNode) + original->len * sizeof(void *);
  LeafNode *inc = RRB_MALLOC(size + sizeof(void *));
  memcpy(inc, original, size);
  inc->len++;
  return inc;
}

static LeafNode* leaf_node_dec(const LeafNode *original) {
  size_t size = sizeof(LeafNode) + (original->len - 1) * sizeof(void *);
  LeafNode *dec = RRB_MALLOC(size); // assumes size > 1
  memcpy(dec, original, size);
  dec->len--;
  return dec;
}


static LeafNode* leaf_node_create(uint32_t len) {
  LeafNode *node = RRB_MALLOC(sizeof(LeafNode) + len * sizeof(void *));
  node->type = LEAF_NODE;
  node->len = len;
  return node;
}

static LeafNode* leaf_node_merge(LeafNode *left, LeafNode *right) {
  LeafNode *merged = leaf_node_create(left->len + right->len);

  memcpy(&merged->child[0], left->child, left->len * sizeof(void *));
  memcpy(&merged->child[left->len], right->child, right->len * sizeof(void *));
  return merged;
}

static InternalNode* internal_node_create(uint32_t len) {
  InternalNode *node = RRB_MALLOC(sizeof(InternalNode)
                              + len * sizeof(InternalNode *));
  node->type = INTERNAL_NODE;
  node->len = len;
  node->size_table = NULL;
  return node;
}

static InternalNode* internal_node_new_above1(InternalNode *child) {
  InternalNode *above = internal_node_create(1);
  above->child[0] = child;
  return above;
}

static InternalNode* internal_node_new_above(InternalNode *left, InternalNode *right) {
  InternalNode *above = internal_node_create(2);
  above->child[0] = left;
  above->child[1] = right;
  return above;
}

static InternalNode* internal_node_merge(InternalNode *left, InternalNode *centre,
                                         InternalNode *right) {
  // If internal node is NULL, its size is zero.
  uint32_t left_len = (left == NULL) ? 0 : left->len - 1;
  uint32_t centre_len = (centre == NULL) ? 0 : centre->len;
  uint32_t right_len = (right == NULL) ? 0 : right->len - 1;

  InternalNode *merged = internal_node_create(left_len + centre_len + right_len);
  if (left_len != 0) { // memcpy'ing zero elements from/to NULL is undefined.
    memcpy(&merged->child[0], left->child,
           left_len * sizeof(InternalNode *));
  }
  if (centre_len != 0) { // same goes here
    memcpy(&merged->child[left_len], centre->child,
           centre_len * sizeof(InternalNode *));
  }
  if (right_len != 0) { // and here
    memcpy(&merged->child[left_len + centre_len], &right->child[1],
           right_len * sizeof(InternalNode *));
  }

  return merged;
}

static InternalNode* internal_node_clone(const InternalNode *original) {
  size_t size = sizeof(InternalNode) + original->len * sizeof(InternalNode *);
  InternalNode *clone = RRB_MALLOC(size);
  memcpy(clone, original, size);
  return clone;
}

static InternalNode* internal_node_copy(InternalNode *original, uint32_t start,
                                        uint32_t len){
  InternalNode *copy = internal_node_create(len);
  memcpy(copy->child, &original->child[start], len * sizeof(InternalNode *));
  return copy;
}

static InternalNode* internal_node_inc(const InternalNode *original) {
  size_t size = sizeof(InternalNode) + original->len * sizeof(InternalNode *);
  InternalNode *incr = RRB_MALLOC(size + sizeof(InternalNode *));
  memcpy(incr, original, size);
  // update length
  if (incr->size_table != NULL) {
    incr->size_table = size_table_inc(incr->size_table, incr->len);
  }
  incr->len++;
  return incr;
}

static InternalNode* internal_node_dec(const InternalNode *original) {
  size_t size = sizeof(InternalNode) + (original->len - 1) * sizeof(InternalNode *);
  InternalNode *clone = RRB_MALLOC(size);
  memcpy(clone, original, size);
  // update length
  clone->len--;
  // Leaks the size table, but it's okay: Would cost more to actually make a
  // smaller one as its size would be roughly the same size.
  return clone;
}


static InternalNode* rebalance(InternalNode *left, InternalNode *centre,
                               InternalNode *right, uint32_t shift,
                               char is_top) {
  InternalNode *all = internal_node_merge(left, centre, right);
  // top_len is children count of the internal node returned.
  uint32_t top_len; // populated through pointer manipulation.

  uint32_t *node_count = create_concat_plan(all, &top_len);

  InternalNode *new_all = execute_concat_plan(all, node_count, top_len, shift);
  if (top_len <= RRB_BRANCHING) {
    if (is_top == false) {
      return internal_node_new_above1(set_sizes(new_all, shift));
    }
    else {
      return new_all;
    }
  }
  else {
    InternalNode *new_left = internal_node_copy(new_all, 0, RRB_BRANCHING);
    InternalNode *new_right = internal_node_copy(new_all, RRB_BRANCHING,
                                                 top_len - RRB_BRANCHING);
    return internal_node_new_above(set_sizes(new_left, shift),
                                   set_sizes(new_right, shift));
  }
}

/**
 * create_concat_plan takes in the large concatenated internal node and a
 * pointer to an uint32_t, which will contain the reduced size of the rebalanced
 * node. It returns a plan as an array of uint32_t's, and modifies the input
 * pointer to contain the length of said array.
 */

static uint32_t* create_concat_plan(InternalNode *all, uint32_t *top_len) {
  uint32_t *node_count = RRB_MALLOC_ATOMIC(all->len * sizeof(uint32_t));

  uint32_t total_nodes = 0;
  for (uint32_t i = 0; i < all->len; i++) {
    const uint32_t size = all->child[i]->len;
    node_count[i] = size;
    total_nodes += size;
  }

  const uint32_t optimal_slots = ((total_nodes-1) / RRB_BRANCHING) + 1;

  uint32_t shuffled_len = all->len;
  uint32_t i = 0;
  while (optimal_slots + RRB_EXTRAS < shuffled_len) {

    // Skip over all nodes satisfying the invariant.
    while (node_count[i] > RRB_BRANCHING - RRB_INVARIANT) {
      i++;
    }

    // Found short node, so redistribute over the next nodes
    uint32_t remaining_nodes = node_count[i];
    do {
      const uint32_t min_size = MIN(remaining_nodes + node_count[i+1], RRB_BRANCHING);
      node_count[i] = min_size;
      remaining_nodes = remaining_nodes + node_count[i+1] - min_size;
      i++;
    } while (remaining_nodes > 0);

    // Shuffle up remaining node sizes
    for (uint32_t j = i; j < shuffled_len - 1; j++) {
      node_count[j] = node_count[j+1]; // Could use memmove here I guess
    }
    shuffled_len--;
    i--;
  }

  *top_len = shuffled_len;
  return node_count;
}

static InternalNode* execute_concat_plan(InternalNode *all, uint32_t *node_size,
                                         uint32_t slen, uint32_t shift) {
  // the all vector doesn't have sizes set yet.

  InternalNode *new_all = internal_node_create(slen);
  // Current old node index to copy from
  uint32_t idx = 0;

  // Offset is how long into the current old node we've already copied from
  uint32_t offset = 0;

  if (shift == INC_SHIFT(LEAF_NODE_SHIFT)) { // handle leaf nodes here
    for (uint32_t i = 0; i < slen; i++) {
      const uint32_t new_size = node_size[i];
      LeafNode *old = (LeafNode *) all->child[idx];

      if (offset == 0 && new_size == old->len) {
        // just pointer copy the node if there is no offset and both have same
        // size
        idx++;
        new_all->child[i] = (InternalNode *) old;
      }
      else {
        LeafNode *new_node = leaf_node_create(new_size);
        uint32_t cur_size = 0;
        // cur_size is the current size of the new node
        // (the amount of elements copied into it so far)

        while (cur_size < new_size /*&& idx < all->len*/) {
          // the commented out check is verified by create_concat_plan --
          // otherwise the implementation is erroneous!
          const LeafNode *old_node = (LeafNode *) all->child[idx];

          if (new_size - cur_size >= old_node->len - offset) {
            // if this node can contain all elements not copied in the old node,
            // copy all of them into this node
            memcpy(&new_node->child[cur_size], &old_node->child[offset],
                   (old_node->len - offset) * sizeof(void *));
            cur_size += old_node->len - offset;
            idx++;
            offset = 0;
          }
          else {
            // if this node can't contain all the elements not copied in the old
            // node, copy as many as we can and pass the old node over to the
            // new node after this one.
            memcpy(&new_node->child[cur_size], &old_node->child[offset],
                   (new_size - cur_size) * sizeof(void *));
            offset += new_size - cur_size;
            cur_size = new_size;
          }
        }

        new_all->child[i] = (InternalNode *) new_node;
      }
    }
  }
  else { // not at lowest non-leaf level
    // this is ALMOST equivalent with the leaf node copying, the only difference
    // is that this is with internal nodes and the fact that they have to create
    // their size tables.

    // As that's the only difference, I won't bother with comments here.
    for (uint32_t i = 0; i < slen; i++) {
      const uint32_t new_size = node_size[i];
      InternalNode *old = all->child[idx];

      if (offset == 0 && new_size == old->len) {
        idx++;
        new_all->child[i] = old;
      }
      else {
        InternalNode *new_node = internal_node_create(new_size);
        uint32_t cur_size = 0;
        while (cur_size < new_size) {
          const InternalNode *old_node = all->child[idx];

          if (new_size - cur_size >= old_node->len - offset) {
            memcpy(&new_node->child[cur_size], &old_node->child[offset],
                   (old_node->len - offset) * sizeof(InternalNode *));
            cur_size += old_node->len - offset;
            idx++;
            offset = 0;
          }
          else {
            memcpy(&new_node->child[cur_size], &old_node->child[offset],
                   (new_size - cur_size) * sizeof(InternalNode *));
            offset += new_size - cur_size;
            cur_size = new_size;
          }
        }
        set_sizes(new_node, DEC_SHIFT(shift)); // This is where we set sizes
        new_all->child[i] = new_node;
      }
    }
  }
  return new_all;
}

// optimize this away?
static uint32_t find_shift(TreeNode *node) {
  if (node->type == LEAF_NODE) {
    return 0;
  }
  else { // must be internal node
    InternalNode *inode = (InternalNode *) node;
    return RRB_BITS + find_shift((TreeNode *) inode->child[0]);
  }
}

static InternalNode* set_sizes(InternalNode *node, uint32_t shift) {
  uint32_t sum = 0;
  RRBSizeTable *table = size_table_create(node->len);
  const uint32_t child_shift = DEC_SHIFT(shift);

  for (uint32_t i = 0; i < node->len; i++) {
    sum += size_sub_trie((TreeNode *) node->child[i], child_shift);
    table->size[i] = sum;
  }
  node->size_table = table;
  return node;
}

static uint32_t size_sub_trie(TreeNode *node, uint32_t shift) {
  if (shift > LEAF_NODE_SHIFT) {
    InternalNode *internal = (InternalNode *) node;
    if (internal->size_table == NULL) {
      uint32_t len = internal->len;
      uint32_t child_shift = DEC_SHIFT(shift);
      // TODO: for loopify recursive calls
      /* We're not sure how many are in the last child, so look it up */
      uint32_t last_size =
        size_sub_trie((TreeNode *) internal->child[len - 1], child_shift);
      /* We know all but the last ones are filled, and they have child_shift
         elements in them. */
      return ((len - 1) << shift) + last_size;
    }
    else {
      return internal->size_table->size[internal->len - 1];
    }
  }
  else {
    LeafNode *leaf = (LeafNode *) node;
    return leaf->len;
  }
}

#ifdef RRB_TAIL
static inline RRB* rrb_tail_push(const RRB *restrict rrb, const void *restrict elt);

static inline RRB* rrb_tail_push(const RRB *restrict rrb, const void *restrict elt) {
  RRB* new_rrb = rrb_head_clone(rrb);
  LeafNode *new_tail = leaf_node_inc(rrb->tail);
  new_tail->child[new_rrb->tail_len] = elt;
  new_rrb->cnt++;
  new_rrb->tail_len++;
  new_rrb->tail = new_tail;
  return new_rrb;
}

#define TAIL_OPTIMISATION(rrb, elt) if (rrb->tail_len < RRB_BRANCHING) {  \
  return rrb_tail_push(rrb, elt); \
  }

#else
#define TAIL_OPTIMISATION(rrb, elt) /* We explicitly do NOT use a tail. */
#endif

#if defined(DIRECT_APPEND) || defined(RRB_TAIL)
#ifdef RRB_TAIL
static InternalNode** copy_first_k(const RRB *rrb, RRB *new_rrb, const uint32_t k,
                                   const uint32_t tail_size);
#else
static InternalNode** copy_first_k(const RRB *rrb, RRB *new_rrb, const uint32_t k);
#endif

static IF_TAIL(InternalNode**, void**)
  append_empty(InternalNode **to_set, uint32_t pos, uint32_t empty_height);

const RRB* rrb_push(const RRB *restrict rrb, const void *restrict elt) {
  TAIL_OPTIMISATION(rrb, elt);
  RRB *new_rrb = rrb_head_clone(rrb);
  new_rrb->cnt++;

#ifdef RRB_TAIL
  LeafNode *new_tail = leaf_node_create(1);
  new_tail->child[0] = elt;
  new_rrb->tail_len = 1;
  return push_down_tail(rrb, new_rrb, new_tail);
}

// Yes, this is turning into an amazing spaghetti.

static RRB* push_down_tail(const RRB *restrict rrb, RRB *restrict new_rrb,
                           const LeafNode *restrict new_tail) {
  const LeafNode *old_tail = new_rrb->tail;
  new_rrb->tail = new_tail;
  if (rrb->cnt <= RRB_BRANCHING) {
    new_rrb->shift = LEAF_NODE_SHIFT;
    new_rrb->root = (TreeNode *) old_tail;
    return new_rrb;
  }
#else
  // small check if the rrb is empty. Should preferably be incorporated into the
  // algorithm instead of being an extreme edge case.
  if (rrb->cnt == 0) {
    LeafNode *leaf = leaf_node_create(1);
    leaf->child[0] = elt;
    new_rrb->shift = LEAF_NODE_SHIFT;
    new_rrb->root = (TreeNode *) leaf;
    return new_rrb;
  }
#endif
  // Copyable count starts here

  // TODO: Can find last rightmost jump in constant time for pvec subvecs:
  // use the fact that (index & large_mask) == 1 << (RRB_BITS * H) - 1 -> 0 etc.

  uint32_t index = rrb->cnt - IF_TAIL(1, 0);

  uint32_t nodes_to_copy = 0;
  uint32_t nodes_visited = 0;
  uint32_t pos = 0; // pos is the position we insert empty nodes in the bottom
                    // copyable node (or the element, if we can copy the leaf)
  const InternalNode *current = (const InternalNode *) rrb->root;

  uint32_t shift = RRB_SHIFT(rrb);

  // checking all non-leaf nodes (or if tail, all but the lowest two levels)
  while (shift > IF_TAIL(INC_SHIFT(LEAF_NODE_SHIFT), 0)) {
    // calculate child index
    uint32_t child_index;
    if (current->size_table == NULL) {
      // some check here to ensure we're not overflowing the pvec subvec.
      // important to realise that this only needs to be done once in a better
      // impl, the same way the size_table check only has to be done until it's
      // false.
      const uint32_t prev_shift = shift + RRB_BITS;
      if (index >> prev_shift > 0) {
        nodes_visited++; // this could possibly be done earlier in the code.
        goto copyable_count_end;
      }
      child_index = (index >> shift) & RRB_MASK;
      // index filtering is not necessary when the check above is performed at
      // most once.
      index &= ~(RRB_MASK << shift);
    }
    else {
      // no need for sized_pos here, luckily.
      child_index = current->len - 1;
      // Decrement index
      if (child_index != 0) {
        index -= current->size_table->size[child_index-1];
      }
    }
    nodes_visited++;
    if (child_index < RRB_MASK) {
      nodes_to_copy = nodes_visited;
      pos = child_index;
    }

    current = current->child[child_index];
    // This will only happen in a pvec subtree
    if (current == NULL) {
      nodes_to_copy = nodes_visited;
      pos = child_index;

      // if next element we're looking at is null, we can copy all above. Good
      // times.
      goto copyable_count_end;
    }
    shift -= RRB_BITS;
  }
  // if we're here, we're at the leaf node (or lowest non-leaf), which is
  // `current`

  // no need to even use index here: We know it'll be placed at current->len,
  // if there's enough space. That check is easy.
#ifdef RRB_TAIL
  if (shift != 0) {
#endif
    nodes_visited++;
    if (current->len < RRB_BRANCHING) {
      nodes_to_copy = nodes_visited;
      pos = current->len;
    }
#ifdef RRB_TAIL
  }
#endif

 copyable_count_end:
  // GURRHH, nodes_visited is not yet handled nicely. for loop down to get
  // nodes_visited set straight.
  while (shift > IF_TAIL(INC_SHIFT(LEAF_NODE_SHIFT), 0)) {
    nodes_visited++;
    shift -= RRB_BITS;
  }

  // Increasing height of tree.
  if (nodes_to_copy == 0) {
    InternalNode *new_root = internal_node_create(2);
    new_root->child[0] = (InternalNode *) rrb->root;
    new_rrb->root = (TreeNode *) new_root;
    new_rrb->shift = INC_SHIFT(RRB_SHIFT(new_rrb));

    // create size table if the original rrb root has a size table.
    if (rrb->root->type != LEAF_NODE &&
        ((const InternalNode *)rrb->root)->size_table != NULL) {
      RRBSizeTable *table = size_table_create(2);
      table->size[0] = rrb->cnt - IF_TAIL(old_tail->len, 0);
      // If we insert the tail, the old size minus the old tail size will be the
      // amount of elements in the left branch. If there is no tail, the size is
      // just the old rrb-tree.

      table->size[1] = rrb->cnt + IF_TAIL(0, 1);
      // If we insert the tail, the old size would include the tail.
      // Consequently, it has to be the old size. If we have no tail, we append
      // a single element to the old vector, therefore it has to be one more
      // than the original.

      new_root->size_table = table;
    }

    // nodes visited == original rrb tree height. Nodes visited > 0.
#ifdef RRB_TAIL
    InternalNode **to_set = append_empty(&((InternalNode *) new_rrb->root)->child[1], 1,
                                         nodes_visited);
    *to_set = (InternalNode *) old_tail;
#else
    void **to_set = append_empty(&((InternalNode *) new_rrb->root)->child[1], 1,
                                 nodes_visited);
    *to_set = (void *) elt;
#endif
  }
  else {
#ifdef RRB_TAIL
    InternalNode **node = copy_first_k(rrb, new_rrb, nodes_to_copy, old_tail->len);
    InternalNode **to_set = append_empty(node, pos, nodes_visited - nodes_to_copy);
    *to_set = (InternalNode *) old_tail;
#else
    InternalNode **node = copy_first_k(rrb, new_rrb, nodes_to_copy);
    void **to_set = append_empty(node, pos, nodes_visited - nodes_to_copy);
    *to_set = (void *) elt;
#endif
  }

  return new_rrb;
}

// - Height should be shift or height, not max element size
// - copy_first_k returns a pointer to the next pointer to set
// - append_empty now returns a pointer to the *void we're supposed to set

#ifdef RRB_TAIL
static InternalNode** copy_first_k(const RRB *rrb, RRB *new_rrb, const uint32_t k,
                                   const uint32_t tail_size)
#else
static InternalNode** copy_first_k(const RRB *rrb, RRB *new_rrb, const uint32_t k)
#endif
{
  const InternalNode *current = (const InternalNode *) rrb->root;
  InternalNode **to_set = (InternalNode **) &new_rrb->root;
  uint32_t index = rrb->cnt - IF_TAIL(1, 0);
  uint32_t shift = RRB_SHIFT(rrb);

  // Copy all non-leaf nodes first. Happens when shift > RRB_BRANCHING
  uint32_t i = 1;
  while (i <= k IF_TAIL(,&& shift != 0)) {
    // First off, copy current node and stick it in.
    InternalNode *new_current;
    if (i != k) {
      new_current = internal_node_clone(current);
      if (current->size_table != NULL) {
        new_current->size_table = size_table_clone(new_current->size_table, new_current->len);
        new_current->size_table->size[new_current->len-1] += IF_TAIL(tail_size, 1);
      }
    }
    else { // increment size of last elt -- will only happen if we append empties
      new_current = internal_node_inc(current);
      if (current->size_table != NULL) {
        new_current->size_table->size[new_current->len-1] =
          new_current->size_table->size[new_current->len-2] + IF_TAIL(tail_size, 1);
      }
    }
    *to_set = new_current;

    // calculate child index
    uint32_t child_index;
    if (current->size_table == NULL) {
      child_index = (index >> shift) & RRB_MASK;
    }
    else {
      // no need for sized_pos here, luckily.
      child_index = new_current->len - 1;
      // Decrement index
      if (child_index != 0) {
        index -= current->size_table->size[child_index-1];
      }
    }
    to_set = &new_current->child[child_index];
    current = current->child[child_index];

    i++;
    shift -= RRB_BITS;
  }

#ifndef RRB_TAIL
  // check if we need to copy any leaf nodes. This is actually very likely to
  // happen (31/32 amortized)
  if (i == k) {
    // copy and increase leaf node size by one
    LeafNode *leaf = leaf_node_create(current->len + 1);
    memcpy(&leaf->child[0], ((const LeafNode *) current)->child,
           current->len * sizeof(void *));
    *to_set = (InternalNode *) leaf;
  }
#endif
  return to_set;
}

static IF_TAIL(InternalNode**, void**)
  append_empty(InternalNode **to_set, uint32_t pos, uint32_t empty_height) {
  if (0 < empty_height) {
#ifdef RRB_TAIL
    InternalNode *leaf = internal_node_create(1);
#else
    LeafNode *leaf = leaf_node_create(1);
#endif
    InternalNode *empty = (InternalNode *) leaf;
    for (uint32_t i = 1; i < empty_height; i++) {
      InternalNode *new_empty = internal_node_create(1);
      new_empty->child[0] = empty;
      empty = new_empty;
    }
    // this root node must be one larger, otherwise segfault
    *to_set = empty;
    return &leaf->child[0];
  }
#ifdef RRB_TAIL
  else {
    return to_set;
  }
#else
  else {
    return &((*((LeafNode **)to_set))->child[pos]);
  }
#endif
}
#else
const RRB* rrb_push(const RRB *restrict rrb, const void *restrict elt) {
  TAIL_OPTIMISATION(rrb, elt);
  LeafNode *right_root = leaf_node_create(1);
  right_root->child[0] = elt;
  const RRB* right = rrb_head_create((TreeNode *) right_root, 1, 0);
  return rrb_concat(rrb, right);
}
#endif

#undef TAIL_OPTIMISATION

static uint32_t sized_pos(const InternalNode *node, uint32_t *index,
                          uint32_t sp) {
  RRBSizeTable *table = node->size_table;
  uint32_t is = *index >> sp;
  while (table->size[is] <= *index) {
    is++;
  }
  if (is != 0) {
    *index -= table->size[is-1];
  }
  return is;
}

static const InternalNode* sized(const InternalNode *node, uint32_t *index,
                                 uint32_t sp) {
  uint32_t is = sized_pos(node, index, sp);
  return (InternalNode *) node->child[is];
}

void* rrb_nth(const RRB *rrb, uint32_t index) {
  if (index >= rrb->cnt) {
    return NULL;
  }
#ifdef RRB_TAIL
  const uint32_t tail_offset = rrb->cnt - rrb->tail_len;
  if (tail_offset <= index) {
    return rrb->tail->child[index - tail_offset];
  }
#endif
  else {
    const InternalNode *current = (const InternalNode *) rrb->root;
    switch (RRB_SHIFT(rrb)) {
#define DECREMENT RRB_MAX_HEIGHT
#include "decrement.h"
#define WANTED_ITERATIONS DECREMENT
#define REVERSE_I(i) (RRB_MAX_HEIGHT - i - 1)
#define LOOP_BODY(i) case (RRB_BITS * REVERSE_I(i)):                  \
      if (current->size_table == NULL) {                              \
        const uint32_t subidx = (index >> (RRB_BITS * REVERSE_I(i)))  \
                                & RRB_MASK;                           \
        current = current->child[subidx];                             \
      }                                                               \
      else {                                                          \
        current = sized(current, &index, RRB_BITS * REVERSE_I(i));    \
      }
#include "unroll.h"
#undef DECREMENT
#undef REVERSE_I
    case 0:
      return ((const LeafNode *)current)->child[index & RRB_MASK];
    default:
      return NULL;
    }
  }
}

uint32_t rrb_count(const RRB *rrb) {
  return rrb->cnt;
}

void* rrb_peek(const RRB *rrb) {
  return rrb_nth(rrb, rrb->cnt - 1);
}

#ifdef RRB_TAIL
/**
 * Destructively replaces the rightmost leaf as the new tail, discarding the
 * old.
 */
// Note that this is very similar to the direct pop algorithm, which is
// described further down in this file.
static void promote_rightmost_leaf(RRB *new_rrb) {
  // special case empty vector here?

  const InternalNode *current = (const InternalNode *) new_rrb->root;
  InternalNode *path[RRB_MAX_HEIGHT+1];
  path[0] = new_rrb->root;
  uint32_t i = 0, shift = LEAF_NODE_SHIFT;

  // populate path array
  for (i = 0, shift = LEAF_NODE_SHIFT; shift < RRB_SHIFT(new_rrb);
       i++, shift += RRB_BITS) {
    path[i+1] = path[i]->child[path[i]->len-1];
  }

  const uint32_t height = i;
  // Set the leaf node as tail.
  new_rrb->tail = (LeafNode *) path[height];
  new_rrb->tail_len = path[height]->len;
  const uint32_t tail_len = new_rrb->tail_len;

  // last element is now always null, in contrast to direct pop
  path[height] = NULL;

  while (i --> 0) {
    // TODO: First skip will always happen. Can we use that somehow?
    if (path[i+1] == NULL) {
      if (path[i]->len == 1) {
        path[i] = NULL;
      }
      else if (i == 0 && path[i]->len == 2) {
        path[i] = path[i]->child[0];
        new_rrb->shift -= RRB_BITS;
      }
      else {
        path[i] = internal_node_dec(path[i]);
      }
    }
    else {
      path[i] = internal_node_clone(path[i]);
      path[i]->child[path[i]->len-1] = path[i+1];
      if (path[i]->size_table != NULL) {
        path[i]->size_table = size_table_clone(path[i]->size_table, path[i]->len);
        // this line differs, as we remove `tail_len` elements from the trie,
        // instead of just 1 as in the direct pop algorithm.
        path[i]->size_table->size[path[i]->len-1] -= tail_len;
      }
    }
  }

  new_rrb->root = (TreeNode *) path[0];
}
#endif

static RRB* slice_right(const RRB *rrb, const uint32_t right) {
  if (right == 0) {
    return rrb_create();
  }
  else if (right < rrb->cnt) {
#ifdef RRB_TAIL
    const uint32_t tail_offset = rrb->cnt - rrb->tail_len;
    // Can just cut the tail slightly
    if (tail_offset < right) {
      RRB *new_rrb = rrb_head_clone(rrb);
      const uint32_t new_tail_len = right - tail_offset;
      LeafNode *new_tail = leaf_node_create(new_tail_len);
      memcpy(new_tail->child, rrb->tail->child, new_tail_len * sizeof(void *));
      new_rrb->cnt = right;
      new_rrb->tail = new_tail;
      new_rrb->tail_len = new_tail_len;
      return new_rrb;
    }
#endif

    RRB *new_rrb = rrb_mutable_create();
    TreeNode *root = slice_right_rec(&RRB_SHIFT(new_rrb), rrb->root, right - 1,
                                     RRB_SHIFT(rrb), false);
    new_rrb->cnt = right;
    new_rrb->root = root;
#ifdef RRB_TAIL
    // Not sure if this is necessary in this part of the program, due to issues
    // wrt. slice_left and roots without size tables.
    promote_rightmost_leaf(new_rrb);
    new_rrb->tail_len = new_rrb->tail->len;
#endif
    return new_rrb;
  }
  else {
    return (RRB *) rrb;
  }
}

static TreeNode* slice_right_rec(uint32_t *total_shift, const TreeNode *root,
                                 uint32_t right, uint32_t shift,
                                 char has_left) {
  const uint32_t subshift = DEC_SHIFT(shift);
  uint32_t subidx = right >> shift;
  if (shift > LEAF_NODE_SHIFT) {
    const InternalNode *internal_root = (InternalNode *) root;
    if (internal_root->size_table == NULL) {
      TreeNode *right_hand_node =
        slice_right_rec(total_shift,
                        (TreeNode *) internal_root->child[subidx],
                        right - (subidx << shift), subshift,
                        (subidx != 0) | has_left);
      if (subidx == 0) {
        if (has_left) {
          InternalNode *right_hand_parent = internal_node_create(1);
          right_hand_parent->child[0] = (InternalNode *) right_hand_node;
          *total_shift = shift;
          return (TreeNode *) right_hand_parent;
        }
        else { // if (!has_left)
          return right_hand_node;
        }
      }
      else { // if (subidx != 0)
        InternalNode *sliced_root = internal_node_create(subidx + 1);
        memcpy(sliced_root->child, internal_root->child,
               subidx * sizeof(InternalNode *));
        sliced_root->child[subidx] = (InternalNode *) right_hand_node;
        *total_shift = shift;
        return (TreeNode *) sliced_root;
      }
    }
    else { // if (internal_root->size_table != NULL)
      RRBSizeTable *table = internal_root->size_table;
      uint32_t idx = right;

      while (table->size[subidx] <= idx) {
        subidx++;
      }
      if (subidx != 0) {
        idx -= table->size[subidx-1];
      }

      const TreeNode *right_hand_node =
        slice_right_rec(total_shift, internal_root->child[subidx], idx,
                        subshift, (subidx != 0) | has_left);
      if (subidx == 0) {
        if (has_left) {
          // As there is one above us, must place the right hand node in a
          // one-node
          InternalNode *right_hand_parent = internal_node_create(1);
          RRBSizeTable *right_hand_table = size_table_create(1);

          right_hand_table->size[0] = right + 1;
          // TODO: Not set size_table if the underlying node doesn't have a
          // table as well.
          right_hand_parent->size_table = right_hand_table;
          right_hand_parent->child[0] = (InternalNode *) right_hand_node;

          *total_shift = shift;
          return (TreeNode *) right_hand_parent;
        }
        else { // if (!has_left)
          return (TreeNode *) right_hand_node;
        }
      }
      else { // if (subidx != 0)
        InternalNode *sliced_root = internal_node_create(subidx+1);
        RRBSizeTable *sliced_table = size_table_create(subidx+1);

        memcpy(sliced_table->size, table->size, subidx * sizeof(uint32_t));
        sliced_table->size[subidx] = right+1;

        memcpy(sliced_root->child, internal_root->child,
               subidx * sizeof(InternalNode *));
        sliced_root->size_table = sliced_table;
        sliced_root->child[subidx] = (InternalNode *) right_hand_node;

        *total_shift = shift;
        return (TreeNode *) sliced_root;
      }
    }
  }
  else { // if (shift <= RRB_BRANCHING)
    // Just pure copying into a new node
    const LeafNode *leaf_root = (LeafNode *) root;
    LeafNode *left_vals = leaf_node_create(subidx + 1);

    memcpy(left_vals->child, leaf_root->child, (subidx + 1) * sizeof(void *));
    *total_shift = shift;
    return (TreeNode *) left_vals;
  }
}

const RRB* slice_left(RRB *rrb, uint32_t left) {
  if (left >= rrb->cnt) {
    return rrb_create();
  }
  else if (left > 0) {
    const uint32_t remaining = rrb->cnt - left;
#ifdef RRB_TAIL
    // If we slice into the tail, we just need to modify the tail itself
    if (remaining <= rrb->tail_len) {
      LeafNode *new_tail = leaf_node_create(remaining);
      memcpy(new_tail->child, &rrb->tail->child[rrb->tail_len - remaining],
             remaining * sizeof(void *));

      RRB *new_rrb = rrb_mutable_create();
      new_rrb->cnt = remaining;
      new_rrb->tail_len = remaining;
      new_rrb->tail = new_tail;
      return new_rrb;
    }
    // Otherwise, we don't really have to take the tail into consideration.
    // Good!
#endif
    RRB *new_rrb = rrb_mutable_create();
    InternalNode *root = (InternalNode *)
      slice_left_rec(&RRB_SHIFT(new_rrb), rrb->root, left,
                     RRB_SHIFT(rrb), false);
    new_rrb->cnt = remaining;
    new_rrb->root = (TreeNode *) root;

    // Ensure last element in size table is correct size, if the root is an
    // internal node.
    if (new_rrb->shift != LEAF_NODE_SHIFT && root->size_table != NULL) {
      root->size_table->size[root->len-1] = new_rrb->cnt - IF_TAIL(rrb->tail_len, 0);
    }
#ifdef RRB_TAIL
    new_rrb->tail = rrb->tail;
    new_rrb->tail_len = rrb->tail_len;
#endif
    rrb = new_rrb;
  }

#ifdef RRB_TAIL

  // TODO: I think the code below also applies to root nodes where size_table
  // == NULL and (cnt - tail_len) & 0xff != 0, but it may be that this is
  // resolved by slice_right itself. Perhaps not promote in the right slicing,
  // but here instead?

  // This case handles leaf nodes < RRB_BRANCHING size, by redistributing
  // values from the tail into the actual leaf node.
  if (RRB_SHIFT(rrb) == 0 && rrb->root != NULL) {
    // two cases to handle: cnt <= RRB_BRANCHING
    //     and (cnt - tail_len) < RRB_BRANCHING

    if (rrb->cnt <= RRB_BRANCHING) {
      // can put all into a new tail
      LeafNode *new_tail = leaf_node_create(rrb->cnt);

      memcpy(&new_tail->child[0], &((LeafNode *) rrb->root)->child[0],
             rrb->root->len * sizeof(void *));
      memcpy(&new_tail->child[rrb->root->len], &rrb->tail->child[0],
             rrb->tail_len * sizeof(void *));
      rrb->tail_len = rrb->cnt;
      rrb->root = NULL;
      rrb->tail = new_tail;
    }
    // no need for <= here, because if the root node is == rrb_branching, the
    // invariant is kept.
    else if (rrb->cnt - rrb->tail_len < RRB_BRANCHING) {
      // create both a new tail and a new root node
      const uint32_t tail_cut = RRB_BRANCHING - rrb->root->len;
      LeafNode *new_root = leaf_node_create(RRB_BRANCHING);
      LeafNode *new_tail = leaf_node_create(rrb->tail_len - tail_cut);

      memcpy(&new_root->child[0], &((LeafNode *) rrb->root)->child[0],
             rrb->root->len * sizeof(void *));
      memcpy(&new_root->child[rrb->root->len], &rrb->tail->child[0],
             tail_cut * sizeof(void *));
      memcpy(&new_tail->child[0], &rrb->tail->child[tail_cut],
             (rrb->tail_len - tail_cut) * sizeof(void *));

      rrb->tail_len = rrb->tail_len - tail_cut;
      rrb->tail = new_tail;
      rrb->root = (TreeNode *) new_root;
    }
  }
#endif
  return rrb;
}

static TreeNode* slice_left_rec(uint32_t *total_shift, const TreeNode *root,
                                uint32_t left, uint32_t shift,
                                char has_right) {
  const uint32_t subshift = DEC_SHIFT(shift);
  uint32_t subidx = left >> shift;
  if (shift > LEAF_NODE_SHIFT) {
    const InternalNode *internal_root = (InternalNode *) root;
    uint32_t idx = left;
    if (internal_root->size_table == NULL) {
      idx -= subidx << shift;
    }
    else { // if (internal_root->size_table != NULL)
      const RRBSizeTable *table = internal_root->size_table;

      while (table->size[subidx] <= idx) {
        subidx++;
      }
      if (subidx != 0) {
        idx -= table->size[subidx - 1];
      }
    }

    const uint32_t last_slot = internal_root->len - 1;
    const TreeNode *child = (TreeNode *) internal_root->child[subidx];
    TreeNode *left_hand_node =
      slice_left_rec(total_shift, child, idx, subshift,
                     (subidx != last_slot) | has_right);
    if (subidx == last_slot) { // No more slots left
      if (has_right) {
        InternalNode *left_hand_parent = internal_node_create(1);
        const InternalNode *internal_left_hand_node = (InternalNode *) left_hand_node;
        left_hand_parent->child[0] = internal_left_hand_node;

        if (subshift != LEAF_NODE_SHIFT && internal_left_hand_node->size_table != NULL) {
          RRBSizeTable *sliced_table = size_table_create(1);
          sliced_table->size[0] =
            internal_left_hand_node->size_table->size[internal_left_hand_node->len-1];
          left_hand_parent->size_table = sliced_table;
        }
        *total_shift = shift;
        return (TreeNode *) left_hand_parent;
      }
      else { // if (!has_right)
        return left_hand_node;
      }
    }
    else { // if (subidx != last_slot)

      const uint32_t sliced_len = internal_root->len - subidx;
      InternalNode *sliced_root = internal_node_create(sliced_len);

      // TODO: Can shrink size here if sliced_len == 2, using the ambidextrous
      // vector technique w. offset. Takes constant time.

      memcpy(&sliced_root->child[1], &internal_root->child[subidx + 1],
             (sliced_len - 1) * sizeof(InternalNode *));

      const RRBSizeTable *table = internal_root->size_table;

      // TODO: Can check if left is a power of the tree size. If so, all nodes
      // will be completely populated, and we can ignore the size table. Most
      // importantly, this will remove the need to alloc a size table, which
      // increases perf.
      RRBSizeTable *sliced_table = size_table_create(sliced_len);

      if (table == NULL) {
        for (uint32_t i = 0; i < sliced_len; i++) {
          // left is total amount sliced off. By adding in subidx, we get faster
          // computation later on.
          sliced_table->size[i] = (subidx + 1 + i) << shift;
          // NOTE: This doesn't really work properly for top root, as last node
          // may have a higher count than it *actually* has. To remedy for this,
          // the top function performs a check afterwards, which may insert the
          // correct value if there's a size table in the root.
        }
      }
      else { // if (table != NULL)
        memcpy(sliced_table->size, &table->size[subidx],
               sliced_len * sizeof(uint32_t));
      }

      for (uint32_t i = 0; i < sliced_len; i++) {
        sliced_table->size[i] -= left;
      }

      sliced_root->size_table = sliced_table;
      sliced_root->child[0] = (InternalNode *) left_hand_node;
      *total_shift = shift;
      return (TreeNode *) sliced_root;
    }
  }
  else { // if (shift <= RRB_BRANCHING)
    LeafNode *leaf_root = (LeafNode *) root;
    const uint32_t right_vals_len = leaf_root->len - subidx;
    LeafNode *right_vals = leaf_node_create(right_vals_len);

    memcpy(right_vals->child, &leaf_root->child[subidx],
           right_vals_len * sizeof(void *));
    *total_shift = shift;

    return (TreeNode *) right_vals;
  }
}

const RRB* rrb_slice(const RRB *rrb, uint32_t from, uint32_t to) {
  return slice_left(slice_right(rrb, to), from);
}

const RRB* rrb_update(const RRB *restrict rrb, uint32_t index, const void *restrict elt) {
  if (index < rrb->cnt) {
    RRB *new_rrb = rrb_head_clone(rrb);
#ifdef RRB_TAIL
    const uint32_t tail_offset = rrb->cnt - rrb->tail_len;
    if (tail_offset <= index) {
      LeafNode *new_tail = leaf_node_clone(rrb->tail);
      new_tail->child[index - tail_offset] = elt;
      new_rrb->tail = new_tail;
      return new_rrb;
    }
#endif
    InternalNode **previous_pointer = (InternalNode **) &new_rrb->root;
    InternalNode *current = (InternalNode *) rrb->root;
    LeafNode *leaf;
    uint32_t child_index;
    switch (RRB_SHIFT(rrb)) {
#define DECREMENT RRB_MAX_HEIGHT
#include "decrement.h"
#define WANTED_ITERATIONS DECREMENT
#define REVERSE_I(i) (RRB_MAX_HEIGHT - i - 1)
#define LOOP_BODY(i) case (RRB_BITS * REVERSE_I(i)):  \
      current = internal_node_clone(current); \
      *previous_pointer = current; \
      if (current->size_table == NULL) { \
        child_index = (index >> (RRB_BITS * REVERSE_I(i))) & RRB_MASK; \
      } \
      else { \
        child_index = sized_pos(current, &index, RRB_BITS * REVERSE_I(i)); \
      } \
      previous_pointer = &current->child[child_index]; \
      current = current->child[child_index];
#include "unroll.h"
#undef DECREMENT
#undef REVERSE_I
    case 0:
      leaf = (LeafNode *) current;
      leaf = leaf_node_clone(leaf);
      *previous_pointer = (InternalNode *) leaf;
      leaf->child[index & RRB_MASK] = elt;
      return new_rrb;
    default:
      return NULL;
    }
  }
  else {
    return NULL;
  }
}

#ifdef RRB_TAIL
// Also assume direct append
const RRB* rrb_pop(const RRB *rrb) {
  if (rrb->cnt == 1) {
    return rrb_create();
  }
  RRB* new_rrb = rrb_head_clone(rrb);
  new_rrb->cnt--;

  if (rrb->tail_len == 1) {
    promote_rightmost_leaf(new_rrb);
    return new_rrb;
  }
  else {
    LeafNode *new_tail = leaf_node_dec(rrb->tail);
    new_rrb->tail_len--;
    new_rrb->tail = new_tail;
    return new_rrb;
  }
}
#else
#ifdef DIRECT_APPEND
// Direct pop function -- more or less verbatim from A.3 in my thesis.
const RRB* rrb_pop(const RRB *rrb) {
  RRB* new_rrb = rrb_head_clone(rrb);
  new_rrb->cnt--;

  InternalNode *path[RRB_MAX_HEIGHT+1];
  path[0] = new_rrb->root;
  uint32_t i = 0, shift = LEAF_NODE_SHIFT;

  // populate path array
  for (i = 0, shift = LEAF_NODE_SHIFT; shift < RRB_SHIFT(new_rrb);
       i++, shift += RRB_BITS) {
    path[i+1] = path[i]->child[path[i]->len-1];
  }

  const uint32_t height = i;
  if (path[height]->len == 1) { // Leaf node contains only single element
    path[height] = NULL;
  }
  else {
    path[height] = (InternalNode *) leaf_node_dec((LeafNode *) path[height]);
    // Remove last element
  }

  while (i --> 0) { // from i = i - 1 downto (and including) 0
    if (path[i+1] == NULL) {
      if (path[i]->len == 1) {
        path[i] = NULL;
      }
      // optimisation here (lines 25-29 in thesis): Instead of cloning the root
      // node all the time, we avoid cloning it if we know we will discard the
      // cloned root (i.e. the height of the trie shrinks). Avoids a memory
      // allocation.
      else if (i == 0 && path[0]->len == 2) {
        path[i] = path[i]->child[0];
        new_rrb->shift -= RRB_BITS;
      }
      else {
        // slightly different from the thesis here: If the node to copy is null,
        // I shrink the array size. This cannot be done through cloning in C.
        path[i] = internal_node_dec(path[i]);
      }
    }
    else {
      path[i] = internal_node_clone(path[i]);
      path[i]->child[path[i]->len-1] = path[i+1];
      if (path[i]->size_table != NULL) { // this is decrement-size-table*
        // copy and decrement last slot in size table if it exists
        path[i]->size_table = size_table_clone(path[i]->size_table, path[i]->len);
        path[i]->size_table->size[path[i]->len-1]--;
      }
    }
  }

  new_rrb->root = (TreeNode *) path[0];
  return new_rrb;
}
#else
const RRB* rrb_pop(const RRB *rrb) {
  return rrb_slice(rrb, 0, rrb->cnt-1);
}
#endif
#endif


#include "rrb_transients.c"

/******************************************************************************/
/*                    DEBUGGING AND VISUALIZATION METHODS                     */
/******************************************************************************/
#ifdef RRB_DEBUG

// Dot Array impl

static DotArray* dot_array_create() {
  DotArray *arr = RRB_MALLOC(sizeof(DotArray));
  arr->len = 0;
  arr->cap = 32;
  arr->elems = RRB_MALLOC(arr->cap * sizeof(const void *));
  return arr;
}

static char dot_array_contains(const DotArray *arr, const void *elem) {
  for (uint32_t i = 0; i < arr->len; i++) {
    if (arr->elems[i] == elem) {
      return true;
    }
  }
  return false;
}

static void dot_array_add(DotArray *arr, const void *elem) {
  if (!dot_array_contains(arr, elem)) {
    // Grow array if needed
    if (arr->len == arr->cap) {
      arr->cap *= 2;
      arr->elems = RRB_REALLOC(arr->elems, arr->cap * sizeof(const void *));
    }
    arr->elems[arr->len] = elem;
    arr->len++;
  }
}

static int null_counter = 0;

void label_pointer(DotFile dot, const void *node, const char *name) {
  fprintf(dot.file, "  \"%s\";\n", name);
  if (node == NULL) { // latest NIL node will be referred to.
    fprintf(dot.file, "  \"%s\" -> s%d;\n", name, null_counter - 1);
  }
  else {
    fprintf(dot.file, "  \"%s\" -> s%p;\n", name, node);
  }
}

static char dot_file_contains(const DotFile dot, const void *elem) {
  return dot_array_contains(dot.array, elem);
}

static void dot_file_add(DotFile dot, const void *elem) {
  dot_array_add(dot.array, elem);
}

DotFile dot_file_create(char *loch) {
  FILE *file = fopen(loch, "w");
  DotArray *arr = dot_array_create();
  fprintf(file, "digraph g {\n  bgcolor=transparent;\n  node [shape=none];\n");
  DotFile dot_file = {.file = file, .array = arr};
  return dot_file;
}

void dot_file_close(DotFile dot) {
  fprintf(dot.file, "}\n");
  fclose(dot.file);
}

void rrb_to_dot_file(const RRB *rrb, char *loch) {
  DotFile dot = dot_file_create(loch);
  rrb_to_dot(dot, rrb);
  dot_file_close(dot);
}

void rrb_to_dot(DotFile dot, const RRB *rrb) {
  if (!dot_file_contains(dot, rrb)) {
    dot_file_add(dot, rrb);
    fprintf(dot.file,
            "  s%p [label=<\n<table border=\"0\" cellborder=\"1\" "
            "cellspacing=\"0\" cellpadding=\"6\" align=\"center\">\n"
            "  <tr>\n"
            "    <td height=\"36\" width=\"25\">%d</td>\n"
            "    <td height=\"36\" width=\"25\">%d</td>\n"
            "    <td height=\"36\" width=\"25\" port=\"root\"></td>\n"
#ifdef RRB_TAIL
            "    <td height=\"36\" width=\"25\">%d</td>\n"
            "    <td height=\"36\" width=\"25\" port=\"tail\"></td>\n"
#endif
            "  </tr>\n"
            "</table>>];\n",
            rrb, rrb->cnt, rrb->shift, IF_TAIL(rrb->tail_len, 0));
#ifdef RRB_TAIL
    if (rrb->tail == NULL) {
      fprintf(dot.file, "  s%d [label=\"NIL\"];\n", null_counter);
      fprintf(dot.file, "  s%p:tail -> s%d;\n", rrb, null_counter++);
    }
    else {
      fprintf(dot.file, "  s%p:tail -> s%p:body;\n", rrb, rrb->tail);
      // Consider doing the rank=same thing.
      leaf_node_to_dot(dot, rrb->tail);
    }
#endif
    if (rrb->root == NULL) {
      fprintf(dot.file, "  s%p:root -> s%d;\n", rrb, null_counter);
    }
    else {
      fprintf(dot.file, "  s%p:root -> s%p:body;\n", rrb, rrb->root);
    }
    tree_node_to_dot(dot, rrb->root, true);
  }
}

static void tree_node_to_dot(DotFile dot, const TreeNode *root, char print_table) {
  if (root == NULL) {
    fprintf(dot.file, "  s%d [label=\"NIL\"];\n",
            null_counter++);
    return;
  }
  switch (root->type) {
  case LEAF_NODE:
    leaf_node_to_dot(dot, (const LeafNode *) root);
    return;
  case INTERNAL_NODE:
    internal_node_to_dot(dot, (const InternalNode *) root, print_table);
    return;
  }
}

static void internal_node_to_dot(DotFile dot, const InternalNode *root,
                                 char print_table) {
  if (!dot_file_contains(dot, root)) {
    dot_file_add(dot, root);
    fprintf(dot.file,
            "  s%p [label=<\n<table border=\"0\" cellborder=\"1\" "
            "cellspacing=\"0\" cellpadding=\"6\" align=\"center\" port=\"body\">\n"
            "  <tr>\n"
            "    <td height=\"36\" width=\"25\" port=\"table\"></td>\n",
            root);
    for (uint32_t i = 0; i < root->len; i++) {
      fprintf(dot.file, "    <td height=\"36\" width=\"25\" port=\"%d\">%d</td>\n",
              i, i);
    }
    fprintf(dot.file, "  </tr>\n</table>>];\n");


    if (print_table) {
      if (root->size_table == NULL) {
        size_table_to_dot(dot, root);
        fprintf(dot.file, "  {rank=same; s%p; s%d;}\n", root, null_counter - 1);
        fprintf(dot.file, "  s%d -> s%p:table [dir=back];\n",
                null_counter - 1, root);
      }
      else if (!dot_file_contains(dot, root->size_table)) {
        // Only do if size table isn't already placed
        // set rrb node and size table at same rank

        fprintf(dot.file, "  {rank=same; s%p; s%p;}\n", root, root->size_table);
        size_table_to_dot(dot, root);
      }
      if (root->size_table != NULL) {
        // "Hack" to get nodes at correct position
        fprintf(dot.file, "  s%p:last -> s%p:table [dir=back];\n",
                root->size_table, root);
      }

    }

    for (uint32_t i = 0; i < root->len; i++) {
      if (root->child[i] == NULL) {
        fprintf(dot.file, "  s%p:%d -> s%d;\n", root, i, null_counter);
      }
      else {
        fprintf(dot.file, "  s%p:%d -> s%p:body;\n", root, i, root->child[i]);
      }
      tree_node_to_dot(dot, (TreeNode *) root->child[i], print_table);
    }
  }
}


static void size_table_to_dot(DotFile dot, const InternalNode *node) {
  if (node->size_table == NULL) {
    fprintf(dot.file, "  s%d [color=indianred, label=\"NIL\"];\n",
            null_counter++);
    return;
  }
  if (!dot_file_contains(dot, node->size_table)) {
    dot_file_add(dot, node->size_table);
    RRBSizeTable *table = node->size_table;
    fprintf(dot.file,
            "  s%p [color=indianred, label=<\n"
            "<table border=\"0\" cellborder=\"1\" cellspacing=\"0\" "
            "cellpadding=\"6\" align=\"center\" port=\"body\">\n"
            "  <tr>\n",
            table);
    for (uint32_t i = 0; i < node->len; i++) {
      int remaining_nodes = (i+1) < node->len;
      fprintf(dot.file, "    <td height=\"36\" width=\"25\" %s>%d</td>\n",
              !remaining_nodes ? "port=\"last\"" : "",
              table->size[i]);
    }
    fprintf(dot.file, "  </tr>\n</table>>];\n");
  }
}

static void leaf_node_to_dot(DotFile dot, const LeafNode *root) {
  if (!dot_file_contains(dot, root)) {
    dot_file_add(dot, root);
    fprintf(dot.file,
            "  s%p [color=darkolivegreen3, label=<\n"
            "<table border=\"0\" cellborder=\"1\" cellspacing=\"0\" "
            "cellpadding=\"6\" align=\"center\" port=\"body\">\n"
            "  <tr>\n",
            root);
    for (uint32_t i = 0; i < root->len; i++) {
      uintptr_t leaf = (uintptr_t) root->child[i];
      fprintf(dot.file, "    <td height=\"36\" width=\"25\">%lx</td>\n",
              leaf);
    }
    fprintf(dot.file, "  </tr>\n</table>>];\n");
  }
}

static uint32_t node_size(DotArray *set, const TreeNode *root) {
  if (root == NULL || dot_array_contains(set, (const void *) root)) {
    return 0;
  }
  dot_array_add(set, (const void *) root);
  switch (root->type) {
  case LEAF_NODE: {
    const LeafNode *leaf = (const LeafNode *) root;
    return sizeof(LeafNode) + sizeof(void *) * leaf->len;
  }
  case INTERNAL_NODE: {
    const InternalNode *internal = (const InternalNode *) root;
    uint32_t size_table_bytes = 0;
    if (internal->size_table != NULL) {
      size_table_bytes = sizeof(uint32_t) * internal->len;
    }
    uint32_t node_bytes = sizeof(InternalNode) + size_table_bytes
                        + sizeof(struct InternalNode *) * internal->len;
    for (uint32_t i = 0; i < internal->len; i++) {
      node_bytes += node_size(set, (const TreeNode *) internal->child[i]);
    }
    return node_bytes;
  }
  }
}

// Need to use GCC for using this. Clang cannot handle vararg -g-symbols
// correctly (yet)
void nodes_to_dot_file(char *loch, int ncount, ...) {
  va_list nodes;
  DotFile dot = dot_file_create(loch);

  va_start(nodes, ncount);
  for (int i = 0; i < ncount; i++) {
    tree_node_to_dot(dot, va_arg(nodes, TreeNode*), true);
  }
  va_end(nodes);
  dot_file_close(dot);
}

static void validate_subtree(const TreeNode *root, uint32_t expected_size,
                             uint32_t root_shift, uint32_t *fail) {
  if (root_shift == LEAF_NODE_SHIFT) { // leaf node
    if (root->type != LEAF_NODE) {
      puts("Expected this node to be a leaf node, but it claims to be "
           "something else.\n Will treat it like a leaf node, "
           "so may segfault.");
      *fail = 1;
    }
    const LeafNode *leaf = (const LeafNode *) root;
    if (leaf->len != expected_size) {
      printf("Leaf node claims to be %u elements long, but was expected to be "
             "%u\n elements long. Will attempt to read %u elements.\n",
             leaf->len, expected_size, MAX(leaf->len, expected_size));
      *fail = 1;
    }
    uintptr_t c = 0;
    // dummy counter to avoid optimization at lower levels (although probably
    // unneccesary). Note that this will probably be filtered out with -O2 and
    // higher, so run with '-O0 -g'.
    for (uint32_t i = 0; i < MAX(leaf->len, expected_size); i++) {
      c += (uintptr_t) leaf->child[i];
    }
  }
  else {
    if (root->type != INTERNAL_NODE) {
      puts("Expected this node to be an internal node, but it claims to be "
           "something else.\n Will treat it like an internal node, "
           "so may segfault.");
      *fail = 1;
    }
    const InternalNode *internal = (const InternalNode *) root;
    if (internal->size_table != NULL) {
      // expected size should be consistent with what's in the last size table
      // slot
      if (internal->size_table->size[internal->len-1] != expected_size) {
        printf("Expected subtree to be of size %u, but its size table says it "
               "is %u.\n", expected_size,
               internal->size_table->size[internal->len-1]);
        *fail = 1;
      }
      for (uint32_t i = 0; i < internal->len; i++) {
        uint32_t size_sub_trie = internal->size_table->size[i]
                               - (i == 0 ? 0 : internal->size_table->size[i-1]);
        validate_subtree((const TreeNode *) internal->child[i], size_sub_trie,
                         DEC_SHIFT(root_shift), fail);
      }
    }
    else { // internal->size_table == NULL
      // this tree may contain at most (internal->len << shift) elements, not
      // more. Effectively, the tree contains (len - 1) << shift + last_tree_len
      // (1 << shift) >= last_tree_len > 0
      const uint32_t child_shift = DEC_SHIFT(root_shift);
      const uint32_t child_max_size = 1 << root_shift;

      if (expected_size > internal->len * child_max_size) {
        printf("Expected size (%u) is larger than what can possibly be inside "
               "this subtree: %u.\n", expected_size,
               internal->len * child_max_size);
        *fail = 1;
      }
      else if (expected_size < ((internal->len - 1) * child_max_size)) {
        printf("Expected size (%u) is smaller than %u, implying that some "
               "non-rightmost node\n is not completely populated.\n",
               expected_size, ((internal->len - 1) << root_shift));
        *fail = 1;
      }
      for (uint32_t i = 0; i < internal->len - 1; i++) {
        validate_subtree((const TreeNode *) internal->child[i], child_max_size,
                         child_shift, fail);
      }
      validate_subtree((const TreeNode *) internal->child[internal->len - 1],
                       expected_size - ((internal->len - 1) * child_max_size),
                       child_shift, fail);
    }
  }
}

uint32_t validate_rrb(const RRB *rrb) {
  // ensure the rrb tree is consistent
  uint32_t fail = 0;
#ifdef RRB_TAIL
  // the rrb tree should always have a tail
  if (rrb->tail->len != rrb->tail_len) {
    fail = 1;
    printf("The tail of this rrb-tree says it is of length %u, but the rrb head"
           "claims it\nis %u elements long.", rrb->tail->len, rrb->tail_len);
  }
  else {
    validate_subtree((TreeNode *) rrb->tail, rrb->tail_len, LEAF_NODE_SHIFT,
                     &fail);
  }
#endif
  if (rrb->root == NULL) {
    if (rrb->cnt - IF_TAIL(rrb->tail_len, 0) != 0) {
      printf("Root is null, but the size of the vector "
             IF_TAIL("(excluding its tail) ","") "is %u.\n",
             rrb->cnt - IF_TAIL(rrb->tail_len, 0));
      fail = 1;
    }
  }
  else {
    validate_subtree(rrb->root, rrb->cnt - IF_TAIL(rrb->tail_len, 0),
                      rrb->shift, &fail);
  }
  return fail;
}

uint32_t rrb_memory_usage(const RRB *const *rrbs, uint32_t rrb_count) {
  DotArray *set = dot_array_create();
  uint32_t sum = 0;
  for (uint32_t i = 0; i < rrb_count; i++) {
    if (!dot_array_contains(set, (const void *) rrbs[i])) {
      dot_array_add(set, (const void *) rrbs[i]);
      sum += sizeof(RRB) + node_size(set, rrbs[i]->root);
#ifdef RRB_TAIL
      sum += node_size(set, rrbs[i]->tail);
#endif
    }
  }
  return sum;
}

#endif
