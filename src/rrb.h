#ifndef RRB_H
#define RRB_H

#include <stdint.h>

#define RRB_BITS 1
#define RRB_BRANCHING (1 << RRB_BITS)
#define RRB_MASK (RRB_BITS - 1)

typedef struct RRB {
  const uint32_t cnt;
} RRB;

RRB* rrb_create(void);
void rrb_destroy(RRB *restrict rrb);

uint32_t rrb_count(const RRB *restrict rrb);
void* rrb_nth(const RRB *restrict rrb, uint32_t pos);
RRB* rrb_pop(const RRB *restrict rrb);
void* rrb_peek(const RRB *restrict rrb);
RRB* rrb_push(const RRB *restrict rrb, const void *restrict elt);
RRB* rrb_update(const RRB *restrict rrb, uint32_t pos, void *restrict elt);

// TODO: Is it okay to use restrict here?
RRB* rrb_concat(const RRB *first, const RRB *second);
RRB* rrb_slice(const RRB *restrict rrb, uint32_t from, uint32_t to);

#endif
