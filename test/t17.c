

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "glme.h"

// Structures without helper macros

struct other
{
  unsigned int u;
  int i;
};

int encode_struct_other(glme_buf_t *enc, const void *ptr)
{
  int e, delta = 1;
  size_t not_empty;
  size_t nc = glme_buf_len(enc);
  const struct other *p = (const struct other *)ptr;

  // first field
  not_empty = p->u != 0;
  e = glme_encode_field(enc, &delta, GLME_UINT, GLME_F_NONE,
			&p->u, 0, not_empty, (encoder)glme_encode_uint);
  if (e < 0) return e;

  not_empty = p->i != 0; 
  e = glme_encode_field(enc, &delta, GLME_INT, GLME_F_NONE,
			&p->i, 0, not_empty, (encoder)glme_encode_int);
  if (e < 0) return e;

  e = glme_encode_end_struct(enc);
  return e < 0 ? e : enc->count - nc;
}

int decode_struct_other(glme_buf_t *enc, void *ptr)
{
  int e, delta = 1;
  size_t nc = glme_buf_len(enc);
  struct other *p = (struct other *)ptr;

  // first field
  p->u = 0;
  e =glme_decode_field(enc, &delta, GLME_UINT, GLME_F_NONE,
		       &p->u, 0, sizeof(int), (decoder)glme_decode_uint);

  p->i = 0; 
  e = glme_decode_field(enc, &delta, GLME_INT, GLME_F_NONE,
			&p->i, 0, sizeof(int), (decoder)glme_decode_int);

  e = glme_decode_end_struct(enc);
  return enc->count - nc;
}

struct test
{
  int a;
  double b;
  char vec[4];
  char *s;
  unsigned int uvec[3];
  size_t ilen;
  int *iv;
  struct other other;
  struct other *optr;
};

int encode_struct_test(glme_buf_t *enc, const void *ptr)
{
  int n, delta = 1;
  size_t nlen;
  size_t not_empty;
  size_t nc = glme_buf_len(enc);
  const struct test *p = (const struct test *)ptr;

  // first field
  not_empty = p->a != 0;
  glme_encode_field(enc, &delta, GLME_INT, GLME_F_NONE,
		    &p->a, 0, not_empty, (encoder)glme_encode_int);

  not_empty = p->b != 0.0; 
  glme_encode_field(enc, &delta, GLME_FLOAT, 0,
		    &p->b, 0, not_empty, (encoder)glme_encode_double);

  nlen = 4;
  not_empty = p->vec[0] != '\0' ? 1 : 0;
  glme_encode_field(enc, &delta, GLME_VECTOR, 0,
		    p->vec, sizeof(p->vec), not_empty, (encoder)0);

  // if .s is null field is omitted
  glme_encode_field(enc, &delta, GLME_STRING, 0, p->s, 0, 1, (encoder)0);

  // this field is never omitted
  glme_encode_field(enc, &delta, GLME_UINT, GLME_F_ARRAY, p->uvec, 3,
		    sizeof(unsigned int), (encoder)glme_encode_value_uint);

  // if .iv is null or .ilen == 0 field is omitted
  glme_encode_field(enc, &delta, GLME_INT, GLME_F_ARRAY, p->iv, p->ilen,
		    sizeof(int), (encoder)glme_encode_value_int);

  glme_encode_field(enc, &delta, 33, 0, &p->other, 0, sizeof(p->other),
		    (encoder)encode_struct_other);

  glme_encode_field(enc, &delta, 33, 0, p->optr, 0, sizeof(p->optr[0]),
		    (encoder)encode_struct_other);

  glme_encode_end_struct(enc);

  return glme_buf_len(enc) - nc;
}

int decode_struct_test(glme_buf_t *gb, void *ptr)
{
  int typeid, i, n, delta = 1;
  size_t nlen;
  size_t nc = glme_buf_at(gb);
  struct test *p = (struct test *)ptr;

  p->a = 0;
  n = glme_decode_field(gb, &delta, GLME_INT, 0, &p->a, &nlen,
			0, (decoder)glme_decode_int);
  if (n < 0) return -10;

  p->b = 0.0;
  n = glme_decode_field(gb, &delta, GLME_FLOAT, 0, &p->b, &nlen,
			0, (decoder)glme_decode_double);
  if (n < 0) return -11;

  ptr = &p->vec[0]; nlen = 4;
  memset(p->vec, 0, sizeof(p->vec));
  n = glme_decode_field(gb, &delta, GLME_VECTOR, 0, ptr, &nlen, 1, (decoder)0);
  if (n < 0) return -12;

  p->s = (void *)0; nlen = 0;
  n = glme_decode_field(gb, &delta, GLME_STRING, 0, &p->s, &nlen, 1, (decoder)0);
  if (n < 0) return -13;

  ptr = &p->uvec[0]; nlen = 3;
  memset(p->uvec, 0, sizeof(p->uvec));
  n = glme_decode_field(gb, &delta, GLME_UINT, GLME_F_ARRAY, &ptr, &nlen,
			sizeof(int), (decoder)glme_decode_value_uint);
  if (n < 0) return -14;

  p->iv = (int *)0;  p->ilen = 0;
  n = glme_decode_field(gb, &delta, GLME_INT, GLME_F_ARRAY, &p->iv, &p->ilen,
			sizeof(int), (decoder)glme_decode_value_int);
  if (n < 0) return -15;

  nlen = 0;
  n = glme_decode_field(gb, &delta, 33, 0, &p->other, &nlen,
			sizeof(p->other), (decoder)decode_struct_other);
  if (n < 0) return -16;

  nlen = 0;
  n = glme_decode_field(gb, &delta, 33, GLME_F_PTR, &p->optr, &nlen,
			sizeof(p->optr[0]), (decoder)decode_struct_other);
  if (n < 0) return -16;

  if (glme_decode_end_struct(gb) < 0)
    return -4;

  return glme_buf_at(gb) - nc;
}

main(int argc, char *argv)
{
  glme_buf_t gbuf;
  struct test t0, t1, t2, t3;
  int i, n0, n1, typeid, iv[2] = {-1, -2};
  size_t len;
  struct other oval = (struct other){129, -62};

  t0 = (struct test){.a = 10,
		     .b = -17.0,
		     .vec = {/*0*/'w', 'o', 'r', 'l'},
		     .s = /*(char *)0,*/ "hello",
		     .uvec = {5, 6, 7},
		     .ilen = 2,
		     .iv = iv,
		     .other = (struct other){67, -2},
		     .optr = &oval
  };
  t1 = (struct test){.a = 2,
		     .b = -3.0,
		     .vec = {'0', '0', '0', '0'},
		     .s = "abcde",
		     .uvec = {0, 0, 0},
		     .ilen = 0,
		     .iv = (int *)0,
		     .other = (struct other){0, 0},
		     .optr = (struct other *)0
  };

  //t2 = (struct test){0, -2.0};
  //t3 = (struct test){1, -3.0};

  glme_buf_init(&gbuf, 1024);

  glme_encode_struct(&gbuf, 32, (void *)&t0, (encoder)encode_struct_test);
  //encode_struct(&gbuf, &t2);
  if (argc > 1)
    write(1, glme_buf_data(&gbuf), glme_buf_len(&gbuf));

  n0 = glme_decode_struct(&gbuf, 32, &t1, (decoder)decode_struct_test);
  //n1 = decode_struct(&gbuf, &t3);

  assert(t0.a == t1.a);
  assert(t0.b == t1.b);
  fprintf(stderr, "t1.a      : %d\n", t1.a);
  fprintf(stderr, "t1.b      : %.2f\n", t1.b);
  fprintf(stderr, "t1.vec[2] : %c\n", t1.vec[2]);
  fprintf(stderr, "t1.s      : %s\n", t1.s);
  fprintf(stderr, "t1.uvec[1]: %d\n", t1.uvec[1]);
  fprintf(stderr, "t1.ilen   : %ld\n", t1.ilen);
  fprintf(stderr, "t1.iv[1]  : %d\n", t1.iv ? t1.iv[1] : 0);
  fprintf(stderr, "t1.other  : %d, %d\n", t1.other.u, t1.other.i);
  fprintf(stderr, "t1.optr   : %d, %d\n",
	  t1.optr ? t1.optr->u : 0, t1.optr ? t1.optr->i : 0);
  //assert(t2.a == t3.a);
  //assert(t2.b == t3.b);
  return 0;
}

/* Local Variables:
 * indent-tabs-mode: nil
 * End:
 */
