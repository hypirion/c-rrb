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
