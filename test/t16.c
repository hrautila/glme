

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "glme.h"


// Cyclic data structure recursively with helper macros.

struct list
{
  struct link *head;
  struct link *tail;
};

struct link
{
  int a;
  double b;
  struct link *next;
  struct link *prev;
};

int encode_link(glme_buf_t *gb, const void *ptr)
{
  const struct link *lnk = (const struct link *)ptr;
  GLME_ENCODE_STDDEF(gb);
  GLME_ENCODE_STRUCT_START(gb);

  GLME_ENCODE_FLD_INT(gb, lnk->a, 0);
  GLME_ENCODE_FLD_DOUBLE(gb, lnk->b, 0.0);
  GLME_ENCODE_FLD_STRUCT(gb, 33, lnk->next, encode_link);

  GLME_ENCODE_STRUCT_END(gb);
  GLME_ENCODE_RETURN(gb);
}

int decode_link(glme_buf_t *gb, void *ptr)
{
  struct link *lnk = (struct link *)ptr;
  GLME_DECODE_STDDEF(gb);
  GLME_DECODE_STRUCT_START(gb);

  GLME_DECODE_FLD_INT(gb, lnk->a, 0);
  GLME_DECODE_FLD_DOUBLE(gb, lnk->b, 0.0);

  // set previous element from decode buffer user data
  lnk->prev = (struct link *)gb->user;
  // update self to be previous
  gb->user = lnk;

  GLME_DECODE_FLD_STRUCT_PTR(gb, 33, lnk->next, decode_link);

  GLME_DECODE_STRUCT_END(gb);
  GLME_DECODE_RETURN(gb);
}


int encode_list(glme_buf_t *gb, const void *ptr)
{
  const struct list *l = (const struct list *)ptr;
  GLME_ENCODE_STDDEF(gb);
  GLME_ENCODE_STRUCT_START(gb);

  GLME_ENCODE_FLD_STRUCT(gb, 33, l->head, encode_link);
  
  GLME_ENCODE_STRUCT_END(gb);
  GLME_ENCODE_RETURN(gb);
}

int decode_list(glme_buf_t *gb, void *ptr)
{
  struct list *l = (struct list *)ptr;
  // use decode buffer user data to save pointer to previous list element
  gb->user = (void *)0;

  GLME_DECODE_STDDEF(gb);
  GLME_DECODE_STRUCT_START(gb);

  GLME_DECODE_FLD_STRUCT_PTR(gb, 33, l->head, decode_link);
  GLME_DECODE_STRUCT_END(gb);

  // decode buffer user data holds pointer to the last decoded element;
  l->tail = (struct link *)gb->user;
  // 'restore' user pointer
  gb->user = (void *)0;

  GLME_DECODE_RETURN(gb);
}

main(int argc, char *argv)
{
  glme_buf_t gbuf;
  int i;

  struct link *n0, *n1, t0[] = {
    { .a = 1,  .b = -2.0, .next = (struct link *)0, .prev = (struct link *)0}, 
    { .a = 0,  .b = -3.0, .next = (struct link *)0, .prev = (struct link *)0},
    { .a = -2, .b =  0.0, .next = (struct link *)0, .prev = (struct link *)0}
  };

  struct list *lp1, l1, l0 = (struct list){t0, &t0[2]};
  t0[0].next = &t0[1]; t0[1].prev = &t0[0];
  t0[1].next = &t0[2]; t0[2].prev = &t0[1];

  glme_buf_init(&gbuf, 1024);

  glme_encode_struct(&gbuf, 32, &l0, encode_list);

  if (argc > 1)
    write(1, glme_buf_data(&gbuf), glme_buf_len(&gbuf));

  lp1 = &l1;
  glme_decode_struct(&gbuf, 32, (void **)&lp1, 0, decode_list);

  for (i = 0, n0 = l0.head, n1 = l1.head; n1; n0 = n0->next, n1 = n1->next, i++) {
    assert(n0->a == n1->a);
    assert(n0->b == n1->b);
  }
  assert(l0.head->a == l1.head->a);
  assert(l0.tail->a == l1.tail->a);
  assert(l0.head->b == l1.head->b);
  assert(l0.tail->b == l1.tail->b);
  return 0;
}

/*
 * Local Variables:
 * indent-tabs-mode: nil
 * End:
 */
