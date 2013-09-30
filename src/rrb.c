#include <stdlib.h>
#include <stdint.h>
#include "rrb.h"

// An empty node. This one is spacier than others, as it can't be malloc'd.
static rrb_node_t EMPTY_NODE = {.size_table = NULL, .child = {0}};

static inline rrb_node_t *node_create(uint32_t children) {
  rrb_node_t *node = calloc(1, sizeof(uint32_t *)
                            + sizeof(rrb_node_t *) * children);
  return node;
}

rrb_t *rrb_create() {
  return NULL;
}
