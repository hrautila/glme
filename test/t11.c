

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "glme.h"

// Data structure iterativelly without helper macros

struct list
{
  struct link *head;
};

struct link
{
  int a;
  double b;
  struct link *next;
};

int encode_link(glme_buf_t *gb, const void *ptr)
{
  int delta;
  unsigned int flag;
  size_t nc = glme_buf_len(gb);
  const struct link *l = (const struct link *)ptr;

  glme_encode_start_struct(gb, &delta);

  glme_encode_field(gb, &delta, GLME_INT, 0, &l->a, 0,
		    (l->a != 0), (glme_encoder_f)glme_encode_int);
  glme_encode_field(gb, &delta, GLME_FLOAT, 0, &l->b, 0,
		    (l->b != 0.0), (glme_encoder_f)glme_encode_double);
  flag = l->next ? 1 : 0;
  glme_encode_field(gb, &delta, GLME_UINT, 0, &flag, 0,
		    flag, (glme_encoder_f)glme_encode_uint);
  glme_encode_end_struct(gb);

  return glme_buf_len(gb) - nc;
}

int decode_link(glme_buf_t *gb, void *ptr)
{
  int delta;
  unsigned int flag;
  size_t nc = glme_buf_at(gb);
  struct link *l = (struct link *)ptr;

  memset(l, 0, sizeof(struct link));

  glme_decode_start_struct(gb, &delta);
  glme_decode_field(gb, &delta, GLME_INT, 0, &l->a, 0, 0,
		    (glme_decoder_f)glme_decode_int);
  glme_decode_field(gb, &delta, GLME_FLOAT, 0, &l->b, 0, 0,
		    (glme_decoder_f)glme_decode_double);
  glme_decode_field(gb, &delta, GLME_UINT, 0, &l->next, 0,
		    0, (glme_decoder_f)glme_decode_uint64);

  glme_decode_end_struct(gb);
  return glme_buf_at(gb) - nc;
}




int encode_list(glme_buf_t *gb, const void *ptr)
{
  int i, delta;
  unsigned int flag;
  size_t nc = glme_buf_len(gb);
  struct link *n;
  const struct list *l = (const struct list *)ptr;

  glme_encode_start_struct(gb, &delta);

  flag = l->head ? 1 : 0;
  glme_encode_field(gb, &delta, GLME_UINT, 0, &flag, 0,
		    flag, (glme_encoder_f)glme_encode_uint);

  glme_encode_end_struct(gb);
  
  // iterate list and write elements to encoder.
  for (i = 0, n = l->head; n; n = n->next, i++) {
    if (glme_encode_struct(gb, 33, n, encode_link) < 0)
      return -1;
  }
  return glme_buf_len(gb) - nc;
}

int decode_list(glme_buf_t *gb, void *ptr)
{
  int i, typeid, delta = 1;
  size_t nc = glme_buf_len(gb);
  struct link *n, **np;
  struct list *l = (struct list *)ptr;

  glme_decode_start_struct(gb, &delta);

  glme_decode_field(gb, &delta, GLME_UINT, 0, &l->head, 0,
		    0, (glme_decoder_f)glme_decode_uint64);
  glme_decode_end_struct(gb);

  // decode link entries after list head (if head is non-null)
  if (l->head) {
    l->head = (struct link *)0;
    glme_decode_struct(gb, 33, (void **)&l->head, sizeof(struct link), decode_link);

    for (i = 1, n = l->head; n; n = n->next, i++) {
      n->next = (struct link *)0;
      glme_decode_struct(gb, 33, (void **)&n->next, sizeof(struct link), decode_link);
    }
  }

  return glme_buf_len(gb) - nc;
}

main(int argc, char *argv)
{
  glme_buf_t gbuf;
  int e, i, typeid;
  size_t len;

  struct link *n0, *n1, t0[] = {
    { .a = 1,  .b = -2.0, .next = (struct link *)0}, 
    { .a = 0,  .b = -3.0, .next = (struct link *)0},
    { .a = -2, .b =  0.0, .next = (struct link *)0}
  };

  struct list *lp1, l1, l0 = (struct list){t0};
  t0[0].next = &t0[1];
  t0[1].next = &t0[2];

  glme_buf_init(&gbuf, 1024);

  glme_encode_struct(&gbuf, 32, &l0, encode_list);

  if (argc > 1)
    write(1, glme_buf_data(&gbuf), glme_buf_len(&gbuf));

  lp1 = &l1;
  glme_decode_struct(&gbuf, 32, (void **)&lp1, 0, decode_list);
    
  for (i = 0, n0 = l0.head, n1 = l1.head; n1; n0 = n0->next, n1 = n1->next, i++) {
    //fprintf(stderr, "%d: a = %d,%d, b = %f,%f\n", i, n0->a, n1->a, n0->b, n1->b);
    assert(n0->a == n1->a);
    assert(n0->b == n1->b);
  }
  return 0;
}

/* Local Variables:
 * indent-tabs-mode: nil
 * End:
 */
