#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "rrb.h"

static rrb_node_t EMPTY_NODE = {.size_table = NULL,
                                .child = (rrb_node_t *) NULL};

static inline rrb_node_t *node_create(uint32_t children) {
  rrb_node_t *node = calloc(1, sizeof(uint32_t *)
                            + sizeof(rrb_node_t *) * children);
  return node;
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
static rrb_node_t *node_for(const rrb_t *restrict rrb, uint32_t i) {
  if (i < rrb->cnt) {
    rrb_node_t *node = rrb->root;
    for (uint32_t level = rrb->shift; level >= RRB_BITS; level -= RRB_BITS) {
      uint32_t idx = (i >> level) & RRB_MASK; // TODO: don't think we need the mask

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
  }
  // Index out of bounds
  else {
    return NULL; // Or some error later on
  }
}
