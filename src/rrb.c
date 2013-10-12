#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "rrb.h"

typedef struct LeafNode {
  void *child[RRB_BRANCHING];
} LeafNode;

typedef struct RRBSizeTable {
  uint32_t size[RRB_BRANCHING];
} RRBSizeTable;

typedef struct RRBNode {
  RRBSizeTable *size_table;
  struct RRBNode *child[RRB_BRANCHING];
} RRBNode;

typedef struct RealRRB {
  uint32_t cnt;
  uint32_t shift;
  RRBNode *root;
} RealRRB;

static RRBNode EMPTY_NODE = {.size_table = NULL,
                              .child = {(RRBNode *) NULL}};

static RRBNode* node_create() {
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

static void node_swap(RRBNode **from, RRBNode *to) {
  node_unref(*from, 1);
  *from = to;
  node_ref(to);
}

static RRBNode* node_clone(RRBNode *original) {
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

static LeafNode* leaf_node_clone(LeafNode *original) {
  LeafNode *copy = malloc(sizeof(LeafNode));
  memcpy(copy, original, sizeof(LeafNode));
  leaf_node_ref_initialize(copy);
  return copy;
}

static RRBSizeTable* size_table_clone(RRBSizeTable *original) {
  RRBSizeTable *copy = calloc(1, sizeof(RRBSizeTable));
  memcpy(copy, original, sizeof(RRBSizeTable));
  return copy;
}

static RealRRB* rrb_clone(const RealRRB *restrict rrb) {
  RealRRB *newrrb = malloc(sizeof(RealRRB));
  memcpy(newrrb, rrb, sizeof(RealRRB));
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
static LeafNode* node_for(const RealRRB *restrict rrb, uint32_t i) {
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
    uint32_t newpos;
    if (idx != 0) {
      newpos = pos - root->size_table->size[idx - 1];
    }
    else {
      newpos = pos;
    }
    RRBNode *newchild = node_pop(newpos, shift - RRB_BITS, root->child[idx]);
    if (newchild == NULL && idx == 0) {
      return NULL;
    }
    else {
      RRBNode *newroot = node_clone(root);
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
    LeafNode *newroot = leaf_node_clone((LeafNode *) root);
    newroot->child[pos] = NULL;
    return (RRBNode *) newroot;
  }
}

RRB* rrb_pop(const RRB *restrict _rrb) {
  const RealRRB *restrict rrb = (RealRRB *) _rrb;
  switch (rrb->cnt) {
  case 0:
    return NULL;
  case 1:
    return rrb_create();
  default: {
    RealRRB *newrrb = rrb_clone(rrb);
    newrrb->cnt--;
    RRBNode *newroot = node_pop(newrrb->cnt, rrb->shift, rrb->root);

    if (newrrb->shift > 0 && newroot->size_table->size[0] == newrrb->cnt) {
      node_swap(&newrrb->root, newroot->child[0]);
      node_ref(newroot);
      node_unref(newroot, 1);
      newrrb->shift -= RRB_BITS;
    }
    else {
      node_swap(&newrrb->root, newroot);
    }

    return (RRB *) newrrb;
  }
  }
}
