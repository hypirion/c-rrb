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
#include <stdint.h>
#include <string.h>
#include "rrb.h"

#ifdef RRB_DEBUG
#include <stdio.h>
#endif

#define NEW_INDEX_POS(rrbnode, idx, pos) (idx == 0 ? pos : \
                                          rrbnode->size_table->size[idx-1] - pos)

typedef struct LeafNode {
  uint32_t len;
  const void *child[RRB_BRANCHING];
} LeafNode;

typedef struct RRBSizeTable {
  uint32_t size[RRB_BRANCHING];
} RRBSizeTable;

typedef struct RRBNode {
  uint32_t len;
  RRBSizeTable *size_table;
  struct RRBNode *child[RRB_BRANCHING];
} RRBNode;

struct _RRB {
  uint32_t cnt;
  uint32_t shift;
  RRBNode *root;
};

static RRBNode EMPTY_NODE = {.size_table = NULL,
                             .child = {(RRBNode *) NULL}};

static RRBSizeTable ONE_TABLE = {.size = {1, 0}};

static RRBNode* rrb_node_create() {
  RRBNode *node = calloc(1, sizeof(RRBNode));
  return node;
}

static void node_ref_initialize(RRBNode *node) {
  // empty as of now
}

static void node_unref(RRBNode *node, uint32_t shift) {
  // empty as of now
}

static void node_ref(RRBNode *node) {
  // empty as of now
}

static void rrb_node_swap(RRBNode **from, RRBNode *to) {
  node_unref(*from, 1);
  *from = to;
  node_ref(to);
}

static RRBNode* rrb_node_clone(const RRBNode *original) {
  RRBNode *copy = malloc(sizeof(RRBNode));
  memcpy(copy, original, sizeof(RRBNode));
  node_ref_initialize(copy);
  for (int i = 0; i < RRB_BRANCHING && copy->child[i] != NULL; i++) {
    node_ref(copy->child[i]);
  }
  return copy;
}

static void leaf_node_ref_initialize(LeafNode *node) {
  // empty as of now
}

static void leaf_node_unref(LeafNode *node) {
  // empty as of now
}

static void leaf_node_ref(LeafNode *node) {
  // empty as of now
}

static LeafNode* leaf_node_create() {
  LeafNode *node = calloc(1, sizeof(LeafNode));
  return node;
}

static LeafNode* leaf_node_clone(LeafNode *original) {
  LeafNode *copy = malloc(sizeof(LeafNode));
  memcpy(copy, original, sizeof(LeafNode));
  leaf_node_ref_initialize(copy);
  return copy;
}

static RRBSizeTable* size_table_create() {
  RRBSizeTable *table = calloc(1, sizeof(RRBSizeTable));
  return table;
}

static RRBSizeTable* size_table_clone(RRBSizeTable *original) {
  RRBSizeTable *copy = calloc(1, sizeof(RRBSizeTable));
  memcpy(copy, original, sizeof(RRBSizeTable));
  return copy;
}

static void size_table_ref_initialize(RRBSizeTable *table) {
  // empty as of now
}

static void size_table_unref(RRBSizeTable *table) {
  // empty as of now
}

static void size_table_ref(RRBSizeTable *table) {
  // empty as of now
}

static RRB* rrb_clone(const RRB *restrict rrb) {
  RRB *newrrb = malloc(sizeof(RRB));
  memcpy(newrrb, rrb, sizeof(RRB));
  if (newrrb->shift == 0) {
    node_ref(rrb->root);
  }
  else {
    leaf_node_ref((LeafNode *) rrb->root);
  }
  return newrrb;
}

RRB* rrb_create() {
  return NULL;
}

void rrb_destroy(RRB *restrict rrb) {
  return;
}

uint32_t rrb_count(const RRB *restrict rrb) {
  return rrb->cnt;
}

// Iffy, need to pass back the modified i somehow.
static LeafNode* node_for(const RRB *restrict rrb, uint32_t i) {
  if (i < rrb->cnt) {
    RRBNode *node = rrb->root;
    for (uint32_t level = rrb->shift; level >= RRB_BITS; level -= RRB_BITS) {
      uint32_t idx = i >> level; // TODO: don't think we need the mask

      // vv TODO: I *think* this should be sufficient, not 100% sure.
      if (node->size_table->size[idx] <= i) {
        idx++;
        if (node->size_table->size[idx] <= i) {
          idx++;
        }
      }

      if (idx) {
        i -= node->size_table->size[idx];
      }
      node = node->child[idx];
    }
    return (LeafNode *) node;
  }
  // Index out of bounds
  else {
    return NULL; // Or some error later on
  }
}

static RRBNode* node_pop(uint32_t pos, uint32_t shift, RRBNode *root) {
  if (shift > 0) {
    uint32_t idx = pos >> shift;
    if (root->size_table->size[idx] <= pos) {
      idx++;
      if (root->size_table->size[idx] <= pos) {
        idx++;
      }
    }
    uint32_t newpos = NEW_INDEX_POS(root, idx, pos);
    RRBNode *newchild = node_pop(newpos, shift - RRB_BITS, root->child[idx]);
    if (newchild == NULL && idx == 0) {
      return NULL;
    }
    else {
      RRBNode *newroot = rrb_node_clone(root);
      newroot->size_table = size_table_clone(root->size_table);
      newroot->size_table->size[idx]--;
      if (newchild == NULL) {
        node_unref(newroot->child[idx], shift);
        newroot->child[idx] = NULL;
      }
      else {
        rrb_node_swap(&newroot->child[idx], newchild);
      }
      return newroot;
    }
  }
  else if (pos == 0) {
    return NULL;
  }
  else { // shift == 0
    LeafNode *newroot = leaf_node_clone((LeafNode *) root);
    newroot->child[pos] = NULL;
    return (RRBNode *) newroot;
  }
}

RRB* rrb_pop(const RRB *restrict rrb) {
  switch (rrb->cnt) {
  case 0:
    return NULL;
  case 1:
    return rrb_create();
  default: {
    RRB *newrrb = rrb_clone(rrb);
    newrrb->cnt--;
    RRBNode *newroot = node_pop(newrrb->cnt, rrb->shift, rrb->root);

    if (newrrb->shift > 0 && newroot->size_table->size[0] == newrrb->cnt) {
      rrb_node_swap(&newrrb->root, newroot->child[0]);
      node_ref(newroot);
      node_unref(newroot, 1);
      newrrb->shift -= RRB_BITS;
    }
    else {
      rrb_node_swap(&newrrb->root, newroot);
    }

    return (RRB *) newrrb;
  }
  }
}

// Can do this without recursion. Should be faster.
static RRBNode* rrb_new_path(uint32_t shift, const void *elt) {
  if (shift == 0) {
    LeafNode *node = leaf_node_create();
    node->child[0] = elt;
    return (RRBNode *) node;
  }
  else {
    RRBNode *node = rrb_node_create();
    rrb_node_swap(&node->child[0], rrb_new_path(shift - RRB_BITS, elt));
    node->size_table = &ONE_TABLE;
    size_table_ref(&ONE_TABLE);
    return node;
  }
}

static RRBNode *rrb_push_elt(uint32_t pos, uint32_t shift,
                             const RRBNode *parent, const void *restrict elt) {
  uint32_t idx = pos >> shift;
  if (shift == 0) {
    LeafNode *newparent = leaf_node_clone((LeafNode *) parent);
    newparent->child[idx] = elt;
    return (RRBNode *) newparent;
  }
  else {
    RRBNode *newparent = rrb_node_clone(parent);
    RRBNode *child = parent->child[idx];
    if (child != NULL) {
      uint32_t newpos = NEW_INDEX_POS(parent, idx, pos);
      RRBNode *copied_path = rrb_push_elt(newpos, shift - RRB_BITS, child, elt);
      rrb_node_swap(&newparent->child[idx], copied_path);
      // TODO: Update size table
    }
    else {
      RRBNode *generated_path = rrb_new_path(shift - RRB_BITS, elt);
      rrb_node_swap(&newparent->child[idx], generated_path);
    }
    return newparent;
  }
}

RRB* rrb_push(const RRB *restrict rrb, const void *restrict elt) {
  RRB *newrrb = rrb_clone(rrb);
  newrrb->cnt++;
  // overflow root check:
  if (0 /* FIXME */) {
    // Create a new top root, containing the old one
    newrrb->shift += RRB_BITS;
    RRBNode *newroot = rrb_node_create();
    RRBSizeTable *new_size_table = size_table_create();
    rrb_node_swap(&newrrb->root, newroot);
    rrb_node_swap(&newroot->child[0], rrb->root);
    new_size_table->size[0] = rrb->cnt;
    rrb_node_swap(&newroot->child[1], rrb_new_path(rrb->shift, elt));
  }
  // still space in root (or subroot) somewhere
  else {
    RRBNode *newroot = rrb_push_elt(rrb->cnt, rrb->shift, rrb->root, elt);
    rrb_node_swap(&newrrb->root, newroot);
  }
  return (RRB *) newrrb;
}

#ifdef RRB_DEBUG

// refcount not implemented yet:
#define REFCOUNT(...)
REFCOUNT(not, yet, used)

static void rrb_node_to_dot(FILE *out, RRBNode *root, uint32_t shift);
static void leaf_node_to_dot(FILE *out, LeafNode *root);
static void size_table_to_dot(FILE *out, RRBSizeTable *table);

void rrb_to_dot(const RRB *rrb, char *loch) {
  FILE *out = fopen(loch, "w");
  fputs("digraph g {\n  bgcolor=transparent;\n  node [shape=none];\n", out);
  fprintf(out,
          "  s%p [label=<\n<table border=\"0\" cellborder=\"1\" "
          "cellspacing=\"0\" cellpadding=\"6\" align=\"center\">\n"
          "  <tr>\n"
          "    <td height=\"36\" width=\"25\">%d</td>\n"
          "    <td height=\"36\" width=\"25\">%d</td>\n"
          "    <td height=\"36\" width=\"25\" port=\"root\"></td>\n"
          "  </tr>\n"
          "</table>>];\n",
          rrb, rrb->cnt, rrb->shift);
  fprintf(out, "  s%p:root -> s%p:body;\n", rrb, rrb->root);
  rrb_node_to_dot(out, rrb->root, rrb->shift);
  fputs("}\n", out);
  fclose(out);
}

static void rrb_node_to_dot(FILE *out, RRBNode *root, uint32_t shift) {
  if (shift == 0) {
    leaf_node_to_dot(out, (LeafNode *) root);
    return;
  }
  fprintf(out,
          "  s%p [label=<\n<table border=\"0\" cellborder=\"1\" "
          "cellspacing=\"0\" cellpadding=\"6\" align=\"center\" port=\"body\">\n"
          "  <tr>\n"
          "    <td height=\"36\" width=\"25\" port=\"table\"></td>\n",
          root);
  for (int i = 0; i < RRB_BRANCHING && root->child[i] != NULL; i++) {
    fprintf(out, "    <td height=\"36\" width=\"25\" port=\"%d\">%d</td>\n",
            i, i);
  }
  fputs("  </tr>\n</table>>];\n", out);
  // "Hack" to get nodes at correct position
  fprintf(out, "  s%p:last -> s%p:table [dir=back];\n", root->size_table, root);
  // set rrb node and size table at same rank
  fprintf(out, "  {rank=same; s%p; s%p;}\n", root, root->size_table);
  size_table_to_dot(out, root->size_table);
  for (int i = 0; i < RRB_BRANCHING && root->child[i] != NULL; i++) {
    fprintf(out, "  s%p:%d -> s%p:body;\n", root, i, root->child[i]);
    rrb_node_to_dot(out, root->child[i], shift - RRB_BITS);
  }
}

static void size_table_to_dot(FILE *out, RRBSizeTable *table) {
  fprintf(out,
          "  s%p [color=indianred, label=<\n"
          "<table border=\"0\" cellborder=\"1\" cellspacing=\"0\" "
          "cellpadding=\"6\" align=\"center\" port=\"body\">\n"
          "  <tr>\n",
          table);
  for (int i = 0; i < RRB_BRANCHING && table->size[i] != 0; i++) {
    int remaining_nodes = (i+1) < RRB_BRANCHING && table->size[i+1] != NULL;
    fprintf(out, "    <td height=\"36\" width=\"25\" %s>%d</td>\n",
            !remaining_nodes ? "port=\"last\"" : "",
            table->size[i]);
  }
  fputs("  </tr>\n</table>>];\n", out);
}

static void leaf_node_to_dot(FILE *out, LeafNode *root) {
  fprintf(out,
          "  s%p [color=darkolivegreen3, label=<\n"
          "<table border=\"0\" cellborder=\"1\" cellspacing=\"0\" "
          "cellpadding=\"6\" align=\"center\" port=\"body\">\n"
          "  <tr>\n",
          root);
  for (int i = 0; i < RRB_BRANCHING && root->child[i] != NULL; i++) {
    uintptr_t leaf = (uintptr_t) ((void **)root->child)[i];
    fprintf(out, "    <td height=\"36\" width=\"25\">%lx</td>\n",
            leaf);
  }
  fputs("  </tr>\n</table>>];\n", out);
}

#endif
