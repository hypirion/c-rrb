#ifndef RRB_H
#define RRB_H

#include <stdint.h>

#define RRB_BITS 1
#define RRB_BRANCHING (1 << RRB_BITS)
#define RRB_MASK (RRB_BITS - 1)

typedef struct rrb_node_t {
  uint32_t *size_table;
  struct rrb_node_t *child[RRB_BRANCHING];
} rrb_node_t;

typedef struct rrb_t {
  uint32_t cnt;
  uint32_t shift;
  rrb_node_t *root;
} rrb_t;

rrb_t *rrb_create(void);
void rrb_destroy(rrb_t *restrict rrb);

uint32_t rrb_count(const rrb_t *restrict rrb);
void *rrb_nth(const rrb_t *restrict rrb, uint32_t pos);
rrb_t *rrb_pop(const rrb_t *restrict rrb);
void *rrb_peek(const rrb_t *restrict rrb);
rrb_t *rrb_push(const rrb_t *restrict rrb, const void *restrict elt);
rrb_t *rrb_update(const rrb_t *restrict rrb, uint32_t pos, void *restrict elt);

// TODO: Is it okay to use restricted here?
rrb_t *rrb_concat(const rrb_t *first, const rrb_t *second);
rrb_t *rrb_slice(const rrb_t *restrict rrb, uint32_t from, uint32_t to);

#endif
