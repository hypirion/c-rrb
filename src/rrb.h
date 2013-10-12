#ifndef RRB_H
#define RRB_H

#include <stdint.h>

#define RRB_BITS 1
#define RRB_BRANCHING (1 << RRB_BITS)
#define RRB_MASK (RRB_BITS - 1)

typedef struct rrb_tree {
  const uint32_t cnt;
} rrb_tree;

rrb_tree* rrb_create(void);
void rrb_destroy(rrb_tree *restrict rrb);

uint32_t rrb_count(const rrb_tree *restrict rrb);
void* rrb_nth(const rrb_tree *restrict rrb, uint32_t pos);
rrb_tree* rrb_pop(const rrb_tree *restrict rrb);
void* rrb_peek(const rrb_tree *restrict rrb);
rrb_tree* rrb_push(const rrb_tree *restrict rrb, const void *restrict elt);
rrb_tree* rrb_update(const rrb_tree *restrict rrb, uint32_t pos, void *restrict elt);

// TODO: Is it okay to use restrict here?
rrb_tree* rrb_concat(const rrb_tree *first, const rrb_tree *second);
rrb_tree* rrb_slice(const rrb_tree *restrict rrb, uint32_t from, uint32_t to);

#endif
