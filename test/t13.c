

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "glme.h"

struct array
{
  int len;
  int *data;
};

// Structure with array field with helper macros

int encode_array(glme_buf_t *gb, const void *ptr)
{
  const struct array *r = (const struct array *)ptr;
  GLME_ENCODE_STDDEF(gb);
  GLME_ENCODE_STRUCT_START(gb);

  GLME_ENCODE_FLD_INT_ARRAY(gb, r->data, (size_t)r->len, glme_encode_value_int);

  GLME_ENCODE_STRUCT_END(gb);
  GLME_ENCODE_RETURN(gb);
}

int decode_array(glme_buf_t *gb, void *ptr)
{
  struct array *r = (struct array *)ptr;
  size_t len;
  GLME_DECODE_STDDEF(gb);
  GLME_DECODE_STRUCT_START(gb);

  GLME_DECODE_FLD_INT_ARRAY(gb, r->data, len, glme_decode_value_int);
  r->len = len;

  GLME_DECODE_STRUCT_END(gb);
  GLME_DECODE_RETURN(gb);
}

struct darray {
  int len;
  float *data;
};

int encode_darray(glme_buf_t *gb, const void *ptr)
{
  const struct darray *r = (const struct darray *)ptr;
  GLME_ENCODE_STDDEF(gb);
  GLME_ENCODE_STRUCT_START(gb);

  GLME_ENCODE_FLD_FLOAT_ARRAY(gb, r->data, (size_t)r->len, glme_encode_value_float);

  GLME_ENCODE_STRUCT_END(gb);
  GLME_ENCODE_RETURN(gb);
}

int decode_darray(glme_buf_t *gb, void *ptr)
{
  struct darray *r = (struct darray *)ptr;
  size_t len;
  GLME_DECODE_STDDEF(gb);
  GLME_DECODE_STRUCT_START(gb);

  GLME_DECODE_FLD_FLOAT_ARRAY(gb, r->data, len, glme_decode_value_float);
  r->len = len;

  GLME_DECODE_STRUCT_END(gb);
  GLME_DECODE_RETURN(gb);
}

main(int argc, char *argv)
{
  glme_buf_t gbuf;
  int ovec[5], ivec[5] = {1, 2, 3, 4, 5};
  float ofc[5], ifc[5] = {0.2, 1.0, 2.0, 3.0, 4.0};
  int i, n0, n1, typeid;
  size_t len;

  struct array a1, a0 = (struct array){5, ivec};
  struct darray f1, f0 = (struct darray){5, ifc};

  glme_buf_init(&gbuf, 1024);

  glme_encode_struct(&gbuf, 20, &a0, encode_array);
  glme_encode_struct(&gbuf, 21, &f0, encode_darray);

  if (argc > 1)
    write(1, glme_buf_data(&gbuf), glme_buf_len(&gbuf));

  glme_decode_struct(&gbuf, 20, &a1, decode_array);
  glme_decode_struct(&gbuf, 21, &f1, decode_darray);

  assert(a0.len == a1.len);
  assert(memcmp(a0.data, a1.data, sizeof(ivec)) == 0);

  assert(f0.len == f1.len);
  assert(memcmp(f0.data, f1.data, sizeof(ifc)) == 0);
  return 0;
}

/* Local Variables:
 * indent-tabs-mode: nil
 * End:
 */
