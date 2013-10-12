#ifndef RRB_H
#define RRB_H

#include <stdint.h>

#define RRB_BITS 1
#define RRB_BRANCHING (1 << RRB_BITS)
#define RRB_MASK (RRB_BITS - 1)

typedef struct rrb {
  const uint32_t cnt;
} rrb;

rrb* rrb_create(void);
void rrb_destroy(rrb *restrict rrb);

uint32_t rrb_count(const rrb *restrict rrb);
void* rrb_nth(const rrb *restrict rrb, uint32_t pos);
rrb* rrb_pop(const rrb *restrict rrb);
void* rrb_peek(const rrb *restrict rrb);
rrb* rrb_push(const rrb *restrict rrb, const void *restrict elt);
rrb* rrb_update(const rrb *restrict rrb, uint32_t pos, void *restrict elt);

// TODO: Is it okay to use restrict here?
rrb* rrb_concat(const rrb *first, const rrb *second);
rrb* rrb_slice(const rrb *restrict rrb, uint32_t from, uint32_t to);

#endif
