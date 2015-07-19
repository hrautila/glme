

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

// Special encoding/decoding of array field in structure (partial send)

int encode_array(glme_buf_t *gb, const void *ptr)
{
  const struct array *r = (const struct array *)ptr;
  int i;
  size_t len = r->len/2 + 1;
  GLME_ENCODE_STDDEF(gb);
  GLME_ENCODE_STRUCT_START(gb);

  GLME_ENCODE_FLD_INT(gb, r->len, 0);
  GLME_ENCODE_FLD_START_ARRAY(gb, tag1, GLME_INT, len);

  // encode every second element
  for (i = 0; i < len; i++) {
    if (glme_encode_value_int(gb, &r->data[2*i]) < 0)
      return -11;
  }
  GLME_ENCODE_FLD_END_ARRAY(gb, tag1);

  GLME_ENCODE_STRUCT_END(gb);
  GLME_ENCODE_RETURN(gb);
}

int decode_array(glme_buf_t *gb, void *ptr)
{
  struct array *r = (struct array *)ptr;
  int k, len;
  int64_t i64;
  GLME_DECODE_STDDEF(gb);
  GLME_DECODE_STRUCT_START(gb);

  GLME_DECODE_FLD_INT(gb, r->len, 0);
  GLME_DECODE_FLD_START_ARRAY(gb, tag1, GLME_INT, len);

  r->data = (int *)calloc(r->len, sizeof(int));
  if (! r->data)
    return -11;
  for (k = 0; k < len; k++) {
    if (glme_decode_value_int64(gb, &i64) < 0)
      return -10;
    r->data[2*k] = (int)i64;
  }

  GLME_DECODE_FLD_END_ARRAY(gb, tag1);

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
  int i, len = r->len/2 + 1;
  double f64;

  GLME_ENCODE_STDDEF(gb);
  GLME_ENCODE_STRUCT_START(gb);

  GLME_ENCODE_FLD_INT(gb, r->len, 0);
  GLME_ENCODE_FLD_START_ARRAY(gb, tag1, GLME_FLOAT, len);

  // encode every second element
  for (i = 0; i < len; i++) {
    if (glme_encode_value_float(gb, &r->data[2*i]) < 0)
      return -11;
  }
  GLME_ENCODE_FLD_END_ARRAY(gb, tag1);

  GLME_ENCODE_STRUCT_END(gb);
}

int decode_darray(glme_buf_t *gb, void *ptr)
{
  struct darray *r = (struct darray *)ptr;
  int k, len; double f64;
  GLME_DECODE_STDDEF(gb);
  GLME_DECODE_STRUCT_START(gb);

  GLME_DECODE_FLD_INT(gb, r->len, 0);
  GLME_DECODE_FLD_START_ARRAY(gb, tag1, GLME_FLOAT, len);

  r->data = (float *)calloc(r->len, sizeof(float));
  if (! r->data)
    return -11;
  for (k = 0; k < len; k++) {
    if (glme_decode_value_float(gb, &r->data[2*k]) < 0)
      return -11;
  }

  GLME_DECODE_FLD_END_ARRAY(gb, tag1);

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
  assert(memcmp(a0.data, a1.data, sizeof(ivec)) != 0);
  assert(a1.data[1] == 0  && a1.data[3] == 0);
  assert(a1.data[0] == a0.data[0]  && a1.data[2] == a0.data[2] && a1.data[4] == a0.data[4]);

  assert(f0.len == f1.len);
  assert(memcmp(f0.data, f1.data, sizeof(ifc)) != 0);
  assert(f1.data[1] == 0.0  && f1.data[3] == 0.0);
  assert(f1.data[0] == f0.data[0]  && f1.data[2] == f0.data[2] && f0.data[4] == f1.data[4]);
  return 0;
}

/* Local Variables:
 * indent-tabs-mode: nil
 * End:
 */
