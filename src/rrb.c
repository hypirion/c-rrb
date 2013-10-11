#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "rrb.h"

typedef struct leaf_node_t {
  void *child[RRB_BRANCHING];
} leaf_node_t;

static rrb_node_t EMPTY_NODE = {.size_table = NULL,
                                .child = (rrb_node_t *) NULL};

static rrb_node_t *node_create() {
  rrb_node_t *node = calloc(1, sizeof(rrb_node_t));
  return node;
}

static void node_ref_initialize(rrb_node_t *node) {
  // empty as of now
}

static void node_unref(rrb_node_t *node, uint32_t shift) {
  // empty as of now
}

static void node_add_ref(rrb_node_t *node) {
  // if shift == 0, we have a leaf node
  // empty as of now
}

static void node_swap(rrb_node_t **from, rrb_node_t *to) {
  node_unref(*from, 1);
  *from = to;
  node_add_ref(to);
}

static rrb_node_t *node_clone(rrb_node_t *original, uint32_t shift) {
  rrb_node_t *copy = malloc(sizeof(rrb_node_t));
  memcpy(copy, original, sizeof(rrb_node_t));
  node_ref_initialize(copy);
  if (shift != 0) {
    for (int i = 0; i < RRB_BRANCHING && copy->child[i] != NULL; i++) {
      node_add_ref(copy->child[i]);
    }
  }
  return copy;
}

static uint32_t *size_table_clone(uint32_t *original) {
  uint32_t *copy = calloc(RRB_BRANCHING, sizeof(uint32_t));
  memcpy(copy, original, RRB_BRANCHING * sizeof(uint32_t));
  return copy;
}

static rrb_t *rrb_clone(const rrb_t *restrict rrb) {
  rrb_t *newrrb = malloc(sizeof(rrb_t));
  memcpy(newrrb, rrb, sizeof(rrb_t));
  // Memory referencing here.
  return newrrb;
}

rrb_t *rrb_create() {
  return NULL;
}

void rrb_destroy(rrb_t *restrict rrb) {
  return;
}

uint32_t rrb_count(const rrb_t *restrict rrb) {
  return rrb->cnt;
}

// Iffy, need to pass back the modified i somehow.
static leaf_node_t *node_for(const rrb_t *restrict rrb, uint32_t i) {
  if (i < rrb->cnt) {
    rrb_node_t *node = rrb->root;
    for (uint32_t level = rrb->shift; level >= RRB_BITS; level -= RRB_BITS) {
      uint32_t idx = i >> level; // TODO: don't think we need the mask

      // vv TODO: I *think* this should be sufficient, not 100% sure.
      if (node->size_table[idx] <= i) {
        idx++;
        if (node->size_table[idx] <= i) {
          idx++;
        }
      }

      if (idx) {
        i -= node->size_table[idx];
      }
      node = node->child[idx];
    }
    return (leaf_node_t *) node;
  }
  // Index out of bounds
  else {
    return NULL; // Or some error later on
  }
}

static rrb_node_t *node_pop(uint32_t pos, uint32_t shift, rrb_node_t *root) {
  if (shift > 0) {
    uint32_t idx = pos >> shift;
    if (root->size_table[idx] <= pos) {
      idx++;
      if (root->size_table[idx] <= pos) {
        idx++;
      }
    }
    uint32_t newpos;
    if (idx != 0) {
      newpos = pos - root->size_table[idx - 1];
    }
    else {
      newpos = pos;
    }
    rrb_node_t *newchild = node_pop(newpos, shift - RRB_BITS, root->child[idx]);
    if (newchild == NULL && idx == 0) {
      return NULL;
    }
    else {
      rrb_node_t *newroot = node_clone(root, shift);
      newroot->size_table = size_table_clone(root->size_table);
      newroot->size_table[idx]--;
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
    leaf_node_t *newroot = malloc(sizeof(leaf_node_t));
    memcpy(newroot, root, sizeof(leaf_node_t));
    // TODO: Add in refcounting here
    newroot->child[pos] = NULL;
    return (rrb_node_t *) newroot;
  }
}

rrb_t *rrb_pop (const rrb_t *restrict rrb) {
  switch (rrb->cnt) {
  case 0:
    return NULL;
  case 1:
    return rrb_create();
  default: {
    rrb_t *newrrb = rrb_clone(rrb);
    newrrb->cnt--;
    rrb_node_t *newroot = node_pop(newrrb->cnt, rrb->shift, rrb->root);

    if (newrrb->shift > 0 && newroot->size_table[0] == newrrb->cnt) {
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
