
// Copyright (c) Harri Rautila, 2015

#include <stdlib.h>

#ifdef __INLINE__
#undef __INLINE__
#endif

#define __INLINE__

#include "gobber.h"
#include "glme.h"

/*
 * Encode basic type id (0 < id < 32) directly to buffer. 
 */
static inline
int __encode_base_type(glme_buf_t *enc, int id)
{
  unsigned int u = (id << 1);
  if (enc->buflen <= enc->count) {
    if (glme_buf_resize(enc, enc->buflen < 1024 ? enc->buflen : 1024) == 0)
      return -1;
  }
  enc->buf[enc->count] = (unsigned char)u;
  enc->count += 1;
  return 1;
}

// -------------------------------------------------------------------------
// Plain value encoding for base types

int glme_encode_value_uint64(glme_buf_t *enc, const uint64_t *v)
{
  int n, rc = 0;
  if (!enc)
    return 0;
  n = gob_encode_uint64(&enc->buf[enc->count], enc->buflen-enc->count, *v);
  if (n < 0) {
    if (glme_buf_resize(enc, enc->buflen < 1024 ? enc->buflen : 1024) == 0) {
      return -1;
    }
  }
  enc->count += n;
  return n;
}

int glme_encode_value_int64(glme_buf_t *enc, const int64_t *v)
{
  int n, rc = 0;
  if (!enc)
    return 0;
 retry:
  n = gob_encode_int64(&enc->buf[enc->count], enc->buflen-enc->count, *v);
  if (n < 0) {
    if (glme_buf_resize(enc, enc->buflen < 1024 ? enc->buflen : 1024) == 0) {
      return -1;
    }
  }
  enc->count += n;
  return n;
}

int glme_encode_value_double(glme_buf_t *enc, const double *v)
{
  int n;
  if (!enc)
    return 0;
  n = gob_encode_double(&enc->buf[enc->count], enc->buflen-enc->count, *v);
  if (n < 0) {
    if (glme_buf_resize(enc, enc->buflen < 1024 ? enc->buflen : 1024) == 0) {
      return -1;
    }
  }
  enc->count += n;
  return n;
}

int glme_encode_value_complex128(glme_buf_t *enc, const double complex *v)
{
  int n;
  if (!enc)
    return 0;
  n = gob_encode_complex128(&enc->buf[enc->count], enc->buflen-enc->count, *v);
  if (n < 0) {
    if (glme_buf_resize(enc, enc->buflen < 1024 ? enc->buflen : 1024) == 0) {
      return -1;
    }
  }
  enc->count += n;
  return n;
}


int glme_encode_value_ulong(glme_buf_t *gbuf, const unsigned long *v)
{
  uint64_t u64 = (uint64_t)(*v);
  return glme_encode_value_uint64(gbuf, &u64); //(uint64_t)v
}

int glme_encode_value_long(glme_buf_t *gbuf, const long *v)
{
  int64_t u64 = (int64_t)(*v);
  return glme_encode_value_int64(gbuf, &u64); //(int64_t)v);
}

int glme_encode_value_uint(glme_buf_t *gbuf, const unsigned int *v)
{
  uint64_t u64 = (uint64_t)(*v);
  return glme_encode_value_uint64(gbuf, &u64); //(uint64_t)v);
}

int glme_encode_value_int(glme_buf_t *gbuf, const int *v)
{
  int64_t u64 = (int64_t)(*v);
  return glme_encode_value_int64(gbuf, &u64); //(int64_t)v);
}

int glme_encode_value_float(glme_buf_t *gbuf, const float *v)
{
  double d = (double)(*v);
  return glme_encode_value_double(gbuf, &d); //(double)v);
}

int glme_encode_value_complex64(glme_buf_t *gbuf, const float complex *v)
{
  double complex d = (double complex)(*v);
  return glme_encode_value_complex128(gbuf, &d); //(double)v);
}

// -------------------------------------------------------------------------
// Base type encoding functions.

int glme_encode_uint64(glme_buf_t *enc, const uint64_t *v)
{
  int n, nc;
  n = __encode_base_type(enc, GLME_UINT);
  if (n < 0)
    return n;
  nc = glme_encode_value_uint64(enc, v);
  return nc < 0 ? nc : nc + n;
}

int glme_encode_int64(glme_buf_t *enc, const int64_t *v)
{
  int n, nc;
  n = __encode_base_type(enc, GLME_INT);
  if (n < 0)
    return n;
  nc = glme_encode_value_int64(enc, v);
  return nc < 0 ? nc : nc + n;
}

int glme_encode_ulong(glme_buf_t *enc, const unsigned long *v)
{
  int n, nc;
  n = __encode_base_type(enc, GLME_UINT);
  if (n < 0)
    return n;
  nc = glme_encode_value_ulong(enc, v);
  return nc < 0 ? nc : nc + n;
}

int glme_encode_long(glme_buf_t *enc, const long *v)
{
  int n, nc;
  n = __encode_base_type(enc, GLME_INT);
  if (n < 0)
    return n;
  nc = glme_encode_value_long(enc, v);
  return nc < 0 ? nc : nc + n;
}

int glme_encode_uint(glme_buf_t *enc, const unsigned int *v)
{
  int n, nc;
  n = __encode_base_type(enc, GLME_UINT);
  if (n < 0)
    return n;
  nc = glme_encode_value_uint(enc, v);
  return nc < 0 ? nc : nc + n;
}

int glme_encode_int(glme_buf_t *enc, const int *v)
{
  int n, nc;
  n = __encode_base_type(enc, GLME_INT);
  if (n < 0)
    return n;
  nc = glme_encode_value_int(enc, v);
  return nc < 0 ? nc : nc + n;
}

int glme_encode_double(glme_buf_t *enc, const double *v)
{
  int n, nc;
  n = __encode_base_type(enc, GLME_FLOAT);
  if (n < 0)
    return n;
  nc = glme_encode_value_double(enc, v);
  return nc < 0 ? nc : nc + n;
}

int glme_encode_float(glme_buf_t *enc, const float *v)
{
  int n, nc;
  n = __encode_base_type(enc, GLME_FLOAT);
  if (n < 0)
    return n;
  nc = glme_encode_value_float(enc, v);
  return nc < 0 ? nc : nc + n;
}

int glme_encode_complex128(glme_buf_t *enc, const double complex *v)
{
  int n, nc;
  n = __encode_base_type(enc, GLME_COMPLEX);
  if (n < 0)
    return n;
  nc = glme_encode_value_complex128(enc, v);
  return nc < 0 ? nc : nc + n;
}

int glme_encode_complex64(glme_buf_t *enc, const float complex *v)
{
  int n, nc;
  n = __encode_base_type(enc, GLME_COMPLEX);
  if (n < 0)
    return n;
  nc = glme_encode_value_complex64(enc, v);
  return nc < 0 ? nc : nc + n;
}

// --------------------------------------------------------------------
// Byte array types (vectors and strings)

int glme_encode_bytes(glme_buf_t *enc, const void *v, size_t vlen)
{
  char tmp[12];
  int n, rc = 0;
  if (!enc)
    return 0;

  n = gob_encode_uint64(tmp, sizeof(tmp), (uint64_t)vlen);
  if (n + vlen > enc->buflen - enc->count) {
    if (glme_buf_resize(enc, n + vlen - (enc->buflen - enc->count)) == 0)
      return -1;
  }

  memcpy(&enc->buf[enc->count], tmp, n);
  enc->count += n;
  memcpy(&enc->buf[enc->count], v, vlen);
  enc->count += vlen;
  return n+vlen;
}

int glme_encode_string(glme_buf_t *gbuf, const char *s)
{
  int nc, n;
  n = __encode_base_type(gbuf, GLME_STRING);
  if (n < 0)
    return n;
  nc = glme_encode_bytes(gbuf, s, strlen(s));
  if (nc < 0) {
    return nc;
  }
  return n + nc;
}

int glme_encode_vector(glme_buf_t *gbuf, const char *s, size_t len)
{
  int n, nc;
  n = __encode_base_type(gbuf, GLME_VECTOR);
  if (n < 0)
    return n;
  nc = glme_encode_bytes(gbuf, s, len);
  if (nc < 0)
    return nc;
  return n + nc;
}

// -------------------------------------------------------------------------
// Array functions

int glme_encode_array_header(glme_buf_t *gbuf, int typeid, size_t sz)
{
  int n1, n0; 

  if ((n0 = glme_encode_type(gbuf, typeid)) < 0)
    return n0;

  if ((n1 = glme_encode_value_uint64(gbuf, &sz)) < 0)
    return n1;
  return n1 + n0;
}

int glme_encode_array_start(glme_buf_t *enc, int typeid, size_t sz)
{
  int n0; 

  if (__encode_base_type(enc, GLME_ARRAY) < 0)
    return -1;

  if ((n0 = glme_encode_array_header(enc, typeid, sz)) < 0)
    return -1;

  return 1 + n0;
}

int glme_encode_array_data(glme_buf_t *enc, const void *vptr,
                           size_t len, size_t esize, glme_encoder_f efunc)
{
  const char *ptr = (const char *)vptr;
  int k, n;
  size_t i, __at_start = enc->count;

  if (! efunc)
    return -1;

  for (k = 0, i = 0; k < len; k++, i += esize) {
    if ((n = (*efunc)(enc, (const void *)&ptr[i])) < 0)
      return n;
  }
  return enc->count - __at_start;
}

int glme_encode_value_array(glme_buf_t *enc, int typeid, const void *vptr, size_t len,
                            size_t esize, glme_encoder_f efunc)
{
  size_t __at_start = enc->count;

  if (glme_encode_array_header(enc, typeid, len) < 0)
    return -1;

  if (len > 0) {
    if (glme_encode_array_data(enc, vptr, len, esize, efunc) < 0)
      return -1;
  }
  return enc->count - __at_start;
}

int glme_encode_array(glme_buf_t *enc, int typeid,
                      const void *vptr, size_t len, size_t esize, glme_encoder_f efunc)
{
  size_t __at_start = enc->count;

  if (__encode_base_type(enc, GLME_ARRAY) < 0)
    return -1;

  if (glme_encode_array_header(enc, typeid, len) < 0)
    return -1;

  if (vptr && len > 0) {
    if (glme_encode_array_data(enc, vptr, len, esize, efunc) < 0)
      return -1;
  }
  return enc->count - __at_start;
}

int glme_encode_type(glme_buf_t *gbuf, int typeid)
{
  int64_t __t;
  if (typeid <= GLME_BASE_MAX) {
    return __encode_base_type(gbuf, typeid);
  }
  __t = (int64_t)typeid;
  return glme_encode_value_int64(gbuf, &__t);
}

// -------------------------------------------------------------------------
// Structure functions


int glme_encode_start_struct(glme_buf_t *gbuf, int *delta)
{
  if (!delta)
    return -1;
  *delta = 1;
  return 0;
}

int glme_encode_end_struct(glme_buf_t *gbuf)
{
  uint64_t u64 = 0;
  return glme_encode_value_uint64(gbuf, &u64);
}

int glme_encode_field(glme_buf_t *enc, int *delta, int typeid, int flags,
                      const void *vptr, size_t nlen, size_t esize, glme_encoder_f efunc)
{
  int n;
  uint64_t __at_start = enc->count;

  if (! vptr || (vptr && esize == 0)) {
    // empty field; omit from stream and increment delta;
    *delta += 1;
    return 0;
  }

  if (vptr && (flags & GLME_F_ARRAY) && nlen == 0) {
    // zero length array field is omitted
    *delta += 1;
    return 0;
  }
  
  // zero length string is not encoded (is this good as other side is gets null pointer??)
  if (typeid == GLME_STRING && vptr && (*((char *)vptr) == 0)) {
    *delta += 1;
    return 0;
  }
  // what about zero length byte vector?? should it be omitted too?

  if (glme_encode_value_uint(enc, delta) < 0)
    return -1;

  // some simple type or structure
  switch (typeid) {
  case GLME_INT:
  case GLME_UINT:
  case GLME_FLOAT:
    if (!efunc) {
      enc->last_error = GLME_E_NOENC;
      return -1;
    }
    if (flags & GLME_F_ARRAY) {
      // array of (int, uint, float)
      n = glme_encode_array(enc, typeid, vptr, nlen, esize, efunc);
    } else {
      n = (*efunc)(enc, vptr);
    }
    break;

  case GLME_VECTOR:
    n = glme_encode_vector(enc, vptr, nlen);
    break;
      
  case GLME_STRING:
    n = glme_encode_string(enc, (char *)vptr);
    break;

  default:
    if (!efunc) {
      if (!(efunc = glme_get_encoder(enc, typeid))) {
        enc->last_error = GLME_E_NOENC;
        return -1;
      }
    }
    if (flags & GLME_F_ARRAY) {
      n = glme_encode_array(enc, typeid, vptr, nlen, esize, efunc);
    } else {
      if (glme_encode_type(enc, typeid) < 0)
        return -1;
      n = (*efunc)(enc, vptr);
    }
  }
  if (n < 0)
    return -1;
  // set delta to one
  *delta = 1;
  return enc->count - __at_start;
}

int glme_encode_struct(glme_buf_t *enc, int typeid, const void *ptr, glme_encoder_f efunc)
{
  int n;
  uint64_t __at_start = enc->count;

  // if null pointer then no data; only things pointed to are encoded
  if (!ptr)
    return 0;

  if (!efunc) {
    // try to find encoder function
    if (!(efunc = glme_get_encoder(enc, typeid))) {
      enc->last_error = GLME_E_NOENC;
      return -1;
    }
  }

  if (glme_encode_type(enc, typeid) < 0)
    return -1;

  if ((n = (*efunc)(enc, ptr)) < 0)
    return n;

  return enc->count - __at_start;
}

// Local Variables:
// indent-tabs-mode: nil
// End:
