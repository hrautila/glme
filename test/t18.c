

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "glme.h"

// Structures with helper macros

struct other
{
  unsigned int u;
  int i;
};

int encode_struct_other(glme_buf_t *enc, const void *ptr)
{
  const struct other *p = (const struct other *)ptr;
  GLME_ENCODE_STDDEF(enc);
  GLME_ENCODE_STRUCT_START(enc);
  GLME_ENCODE_FLD_UINT(enc, p->u, 0);
  GLME_ENCODE_FLD_INT(enc, p->i, 0);
  GLME_ENCODE_STRUCT_END(enc);
  GLME_ENCODE_RETURN(enc);
}

int decode_struct_other(glme_buf_t *dec, struct other *p)
{
  GLME_DECODE_STDDEF(dec);
  GLME_DECODE_STRUCT_START(dec);
  GLME_DECODE_FLD_UINT(dec, p->u, 0);
  GLME_DECODE_FLD_INT(dec, p->i, 0);
  GLME_DECODE_STRUCT_END(dec);
  GLME_DECODE_RETURN(dec);
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
  const struct test *p = (const struct test *)ptr;

  GLME_ENCODE_STDDEF(enc);
  GLME_ENCODE_STRUCT_START(enc);

  GLME_ENCODE_FLD_INT(enc, p->a, 0);
  GLME_ENCODE_FLD_DOUBLE(enc, p->b, 0.0);
  GLME_ENCODE_FLD_VECTOR(enc, p->vec, sizeof(p->vec));
  GLME_ENCODE_FLD_STRING(enc, p->s);

  GLME_ENCODE_FLD_UINT_VECTOR(enc, p->uvec, glme_encode_value_uint);
  GLME_ENCODE_FLD_INT_ARRAY(enc, p->iv, p->ilen, glme_encode_value_int);
    
  GLME_ENCODE_FLD_STRUCT(enc, 33, &p->other, encode_struct_other);
  GLME_ENCODE_FLD_STRUCT(enc, 33, p->optr, encode_struct_other);

  GLME_ENCODE_STRUCT_END(enc);
  GLME_ENCODE_RETURN(enc);
}

int decode_struct_test(glme_buf_t *dec, void *ptr)
{
  struct test *p = (struct test *)ptr;

  GLME_DECODE_STDDEF(dec);
  GLME_DECODE_STRUCT_START(dec);
  GLME_DECODE_FLD_INT(dec, p->a, 0);
  GLME_DECODE_FLD_DOUBLE(dec, p->b, 0.0);

  GLME_DECODE_FLD_VECTOR(dec, p->vec);
  GLME_DECODE_FLD_STRING(dec, p->s);

  GLME_DECODE_FLD_UINT_VECTOR(dec, p->uvec, glme_decode_value_uint);
  GLME_DECODE_FLD_INT_ARRAY(dec, p->iv, p->ilen, glme_decode_value_int);

  GLME_DECODE_FLD_STRUCT(dec, 33, p->other, decode_struct_other);
  GLME_DECODE_FLD_STRUCT_PTR(dec, 33, p->optr, decode_struct_other);

  GLME_DECODE_STRUCT_END(dec);
  GLME_DECODE_RETURN(dec);
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
