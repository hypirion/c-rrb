#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "rrb.h"

typedef struct leaf_node {
  void *child[RRB_BRANCHING];
} leaf_node;

typedef struct rrb_size_table {
  uint32_t size[RRB_BRANCHING];
} rrb_size_table;

typedef struct rrb_node {
  rrb_size_table *size_table;
  struct rrb_node *child[RRB_BRANCHING];
} rrb_node;

typedef struct real_rrb {
  uint32_t cnt;
  uint32_t shift;
  rrb_node *root;
} real_rrb;

static rrb_node EMPTY_NODE = {.size_table = NULL,
                             .child = (rrb_node *) NULL};

static rrb_node* node_create() {
  rrb_node *node = calloc(1, sizeof(rrb_node));
  return node;
}

static void node_ref_initialize(rrb_node *node) {
  // empty as of now
}

static void node_unref(rrb_node *node, uint32_t shift) {
  // empty as of now
}

static void node_add_ref(rrb_node *node) {
  // if shift == 0, we have a leaf node
  // empty as of now
}

static void node_swap(rrb_node **from, rrb_node *to) {
  node_unref(*from, 1);
  *from = to;
  node_add_ref(to);
}

static rrb_node* node_clone(rrb_node *original, uint32_t shift) {
  rrb_node *copy = malloc(sizeof(rrb_node));
  memcpy(copy, original, sizeof(rrb_node));
  node_ref_initialize(copy);
  if (shift != 0) {
    for (int i = 0; i < RRB_BRANCHING && copy->child[i] != NULL; i++) {
      node_add_ref(copy->child[i]);
    }
  }
  return copy;
}

static rrb_size_table* size_table_clone(rrb_size_table *original) {
  rrb_size_table *copy = calloc(1, sizeof(rrb_size_table));
  memcpy(copy, original, sizeof(rrb_size_table));
  return copy;
}

static real_rrb* rrb_clone(const real_rrb *restrict rrb) {
  real_rrb *newrrb = malloc(sizeof(real_rrb));
  memcpy(newrrb, rrb, sizeof(real_rrb));
  // Memory referencing here.
  return newrrb;
}

rrb* rrb_create() {
  return NULL;
}

void rrb_destroy(rrb *restrict rrb) {
  return;
}

uint32_t rrb_count(const rrb *restrict rrb) {
  return rrb->cnt;
}

// Iffy, need to pass back the modified i somehow.
static leaf_node* node_for(const real_rrb *restrict rrb, uint32_t i) {
  if (i < rrb->cnt) {
    rrb_node *node = rrb->root;
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
    return (leaf_node *) node;
  }
  // Index out of bounds
  else {
    return NULL; // Or some error later on
  }
}

static rrb_node* node_pop(uint32_t pos, uint32_t shift, rrb_node *root) {
  if (shift > 0) {
    uint32_t idx = pos >> shift;
    if (root->size_table->size[idx] <= pos) {
      idx++;
      if (root->size_table->size[idx] <= pos) {
        idx++;
      }
    }
    uint32_t newpos;
    if (idx != 0) {
      newpos = pos - root->size_table->size[idx - 1];
    }
    else {
      newpos = pos;
    }
    rrb_node *newchild = node_pop(newpos, shift - RRB_BITS, root->child[idx]);
    if (newchild == NULL && idx == 0) {
      return NULL;
    }
    else {
      rrb_node *newroot = node_clone(root, shift);
      newroot->size_table = size_table_clone(root->size_table);
      newroot->size_table->size[idx]--;
      if (newchild == NULL) {
        node_unref(newroot->child[idx], shift);
        newroot->child[idx] = NULL;
      }
      else {
        node_swap(&newroot->child[idx], newchild);
      }
      return newroot;
    }
  }
  else if (pos == 0) {
    return NULL;
  }
  else { // shift == 0
    leaf_node *newroot = malloc(sizeof(leaf_node));
    memcpy(newroot, root, sizeof(leaf_node));
    // TODO: Add in refcounting here
    newroot->child[pos] = NULL;
    return (rrb_node *) newroot;
  }
}

rrb* rrb_pop (const rrb *restrict _rrb) {
  const real_rrb *restrict rrb = (real_rrb *) _rrb;
  switch (rrb->cnt) {
  case 0:
    return NULL;
  case 1:
    return rrb_create();
  default: {
    real_rrb *newrrb = rrb_clone(rrb);
    newrrb->cnt--;
    rrb_node *newroot = node_pop(newrrb->cnt, rrb->shift, rrb->root);

    if (newrrb->shift > 0 && newroot->size_table->size[0] == newrrb->cnt) {
      node_swap(&newrrb->root, newroot->child[0]);
      node_add_ref(newroot);
      node_unref(newroot, 1);
      newrrb->shift -= RRB_BITS;
    }
    else {
      node_swap(&newrrb->root, newroot);
    }
    return newrrb;
  }
  }
}
