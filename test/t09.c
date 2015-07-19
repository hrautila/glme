

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "glme.h"

// Structure arrays without helper functions

struct test
{
  int a;
  double b;
};

int encode_struct(glme_buf_t *gb, const void *ptr)
{
  int delta;
  size_t nc = glme_buf_len(gb);
  const struct test *p = (const struct test *)ptr;

  glme_encode_start_struct(gb, &delta);

  glme_encode_field(gb, &delta, GLME_INT, 0, &p->a, 0,
		    (p->a != 0), (encoder)glme_encode_int);
  glme_encode_field(gb, &delta, GLME_FLOAT, 0, &p->b, 0,
		    (p->b != 0.0), (encoder)glme_encode_double);
  glme_encode_end_struct(gb);
  return glme_buf_len(gb) - nc;
}

int decode_struct(glme_buf_t *gb, void *ptr)
{
  int delta;
  size_t nc = glme_buf_at(gb);
  struct test *p = (struct test *)ptr;

  memset(p, 0, sizeof(struct test));

  glme_decode_start_struct(gb, &delta);
  glme_decode_field(gb, &delta, GLME_INT, 0, &p->a, 0, 0,
		    (decoder)glme_decode_int);
  glme_decode_field(gb, &delta, GLME_FLOAT, 0, &p->b, 0, 0,
		    (decoder)glme_decode_double);
  glme_decode_end_struct(gb);
  return glme_buf_at(gb) - nc;
}

main(int argc, char *argv)
{
  glme_buf_t gbuf;
  int i, n0, n1, typeid;
  size_t len;

  struct test t0[] = {
    { .a = 1,  .b = -2.0}, 
    { .a = 0,  .b = -3.0},
    { .a = -2, .b =  0.0}
  };

  struct test t1[] = {
    { .a = 2,  .b = -3.0}, 
    { .a = 1,  .b = -2.0},
    { .a = -3, .b =  1.0}
  };

  glme_buf_init(&gbuf, 1024);

  glme_encode_array_start(&gbuf, 20, 3);
  for (i = 0; i < 3; i++)
    encode_struct(&gbuf, &t0[i]);

  if (argc > 1)
    write(1, glme_buf_data(&gbuf), glme_buf_len(&gbuf));

  glme_decode_array_start(&gbuf, &typeid, &len);
  for (i = 0; i < len; i++)
    decode_struct(&gbuf, &t1[i]);

  assert(typeid == 20);
  assert(len == 3);
  assert(t0[0].a == t1[0].a);
  assert(t0[0].b == t1[0].b);
  assert(t0[1].a == t1[1].a);
  assert(t0[1].b == t1[1].b);
  assert(t0[2].a == t1[2].a);
  assert(t0[2].b == t1[2].b);
  return 0;
}

/* Local Variables:
 * indent-tabs-mode: nil
 * End:
 */
