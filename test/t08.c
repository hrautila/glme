

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "glme.h"

// Structures without helper macros

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
		    (p->a != 0), (glme_encoder_f)glme_encode_int);
  glme_encode_field(gb, &delta, GLME_FLOAT, 0, &p->b, 0,
		    (p->b != 0.0), (glme_encoder_f)glme_encode_double);
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
		    (glme_decoder_f)glme_decode_int);
  glme_decode_field(gb, &delta, GLME_FLOAT, 0, &p->b, 0, 0,
		    (glme_decoder_f)glme_decode_double);
  glme_decode_end_struct(gb);
  return glme_buf_at(gb) - nc;
}

main(int argc, char *argv)
{
  glme_buf_t gbuf;
  struct test t0, t1, t2, t3, *tp1, *tp3;
  int i, n0, n1, typeid;
  size_t len;

  t0 = (struct test){1, -2.0};
  t1 = (struct test){2, -3.0};

  t2 = (struct test){0, -2.0};
  t3 = (struct test){1, -3.0};

  glme_buf_init(&gbuf, 1024);

  glme_encode_struct(&gbuf, 20, &t0, encode_struct);
  glme_encode_struct(&gbuf, 20, &t2, encode_struct);
  if (argc > 1)
    write(1, glme_buf_data(&gbuf), glme_buf_len(&gbuf));

  tp1 = &t1; tp3 = &t3;
  glme_decode_struct(&gbuf, 20, (void **)&tp1, 0, decode_struct);
  glme_decode_struct(&gbuf, 20, (void **)&tp3, 0, decode_struct);

  assert(t0.a == t1.a);
  assert(t0.b == t1.b);
  assert(t2.a == t3.a);
  assert(t2.b == t3.b);
  return 0;
}

/* Local Variables:
 * indent-tabs-mode: nil
 * End:
 */
