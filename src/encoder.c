
// Copyright (c) Harri Rautila, 2015

#include <stdlib.h>
#include "glme/gobber.h"

#ifdef __INLINE__
#undef __INLINE__
#endif

#define __INLINE__

#include "gobber.h"
#include "glme.h"


int glme_encode_uint64(glme_buf_t *enc, uint64_t v)
{
  int n, rc = 0;
  if (!enc)
    return 0;
 retry:
  n = gob_encode_uint64(&enc->buf[enc->count], enc->buflen-enc->count, v);
  if (n > enc->buflen-enc->count) {
    if (rc > 1)
      return -1;
    glme_buf_resize(enc, enc->buflen < 1024 ? enc->buflen : 1024);
    rc++;
    goto retry;
  }
  enc->count += n;
  return n;
}

int glme_encode_int64(glme_buf_t *enc, int64_t v)
{
  int n, rc = 0;
  if (!enc)
    return 0;
 retry:
  n = gob_encode_int64(&enc->buf[enc->count], enc->buflen-enc->count, v);
  if (n > enc->buflen-enc->count) {
    if (rc > 1)
      return -1;
    glme_buf_resize(enc, enc->buflen < 1024 ? enc->buflen : 1024);
    rc++;
    goto retry;
  }
  enc->count += n;
  return n;
}

int glme_encode_double(glme_buf_t *enc, double v)
{
  int n, rc = 0;
  if (!enc)
    return 0;
 retry:
  n = gob_encode_double(&enc->buf[enc->count], enc->buflen-enc->count, v);
  if (n > enc->buflen-enc->count) {
    if (rc > 1)
      return -1;
    glme_buf_resize(enc, enc->buflen < 1024 ? enc->buflen : 1024);
    rc++;
    goto retry;
  }
  enc->count += n;
  return n;
}


int glme_encode_bytes(glme_buf_t *enc, void *v, size_t vlen)
{
  int n, rc = 0;
  if (!enc)
    return 0;
 retry:
  n = gob_encode_uint64(&enc->buf[enc->count], enc->buflen - enc->count, (uint64_t)vlen);
  if (n > enc->buflen - enc->count) {
    if (rc > 1)
      return -1;
    glme_buf_resize(enc, enc->buflen < 1024 ? enc->buflen : 1024);
    rc++;
    goto retry;
  }
  rc = 0;
  enc->count += n;
  while (vlen > enc->buflen - enc->count) {
    if (rc > 1) {
      // readjust count to original state
      enc->count -= n;
      return -1;
    }
    glme_buf_resize(enc, vlen - (enc->buflen - enc->count));
  }
  memcpy(&enc->buf[enc->count], v, vlen);
  enc->count += vlen;
  return n+vlen;
}

int glme_encode_ulong(glme_buf_t *gbuf, unsigned long v)
{
  return glme_encode_uint64(gbuf, (uint64_t)v);
}

int glme_encode_long(glme_buf_t *gbuf, long v)
{
  return glme_encode_int64(gbuf, (int64_t)v);
}

int glme_encode_uint(glme_buf_t *gbuf, unsigned int v)
{
  return glme_encode_uint64(gbuf, (uint64_t)v);
}

int glme_encode_int(glme_buf_t *gbuf, int v)
{
  return glme_encode_int64(gbuf, (int64_t)v);
}

int glme_encode_float(glme_buf_t *gbuf, float v)
{
  return glme_encode_double(gbuf, (double)v);
}

int glme_encode_string(glme_buf_t *gbuf, char *s)
{
  return glme_encode_bytes(gbuf, s, strlen(s)+1);
}

int glme_encode_start_array(glme_buf_t *gbuf, size_t sz)
{
  return glme_encode_uint64(gbuf, (uint64_t)sz);
}

int glme_encode_end_array(glme_buf_t *gbuf, size_t sz)
{
  return 0;
}

int glme_encode_start_struct(glme_buf_t *gbuf, int typeid)
{
  return glme_encode_int64(gbuf, (int64_t)typeid);
}

int glme_encode_end_struct(glme_buf_t *gbuf)
{
  return glme_encode_uint64(gbuf, (uint64_t)0);
}

// Local Variables:
// indent-tabs-mode: nil
// End:
