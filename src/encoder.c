
// Copyright (c) Harri Rautila, 2015

#include <stdlib.h>
#include "glme/gobber.h"

#ifdef __INLINE__
#undef __INLINE__
#endif

#define __INLINE__

#include "glme/encoder.h"

int glme_encoder_writem(glme_encoder_t *enc, int fd)
{
  int n;
  char tmp[16];
  n = gob_encode_uint64(tmp, sizeof(tmp), enc->count);

  if (write(fd, tmp, n) < 0)
    return -1;
  if (write(fd, enc->buf, enc->count) < 0)
    return -1;
  
  return n+enc->count;
}

int glme_encode_uint64(glme_encoder_t *enc, uint64_t v)
{
  int n, rc = 0;
  if (!enc)
    return 0;
 retry:
  n = gob_encode_uint64(&enc->buf[enc->count], enc->buflen-enc->count, v);
  if (n > enc->buflen-enc->count) {
    if (rc > 1)
      return -1;
    glme_encoder_resize(enc, enc->buflen < 1024 ? enc->buflen : 1024);
    rc++;
    goto retry;
  }
  enc->count += n;
  return n;
}

int glme_encode_int64(glme_encoder_t *enc, int64_t v)
{
  int n, rc = 0;
  if (!enc)
    return 0;
 retry:
  n = gob_encode_int64(&enc->buf[enc->count], enc->buflen-enc->count, v);
  if (n > enc->buflen-enc->count) {
    if (rc > 1)
      return -1;
    glme_encoder_resize(enc, enc->buflen < 1024 ? enc->buflen : 1024);
    rc++;
    goto retry;
  }
  enc->count += n;
  return n;
}

int glme_encode_double(glme_encoder_t *enc, double v)
{
  int n, rc = 0;
  if (!enc)
    return 0;
 retry:
  n = gob_encode_double(&enc->buf[enc->count], enc->buflen-enc->count, v);
  if (n > enc->buflen-enc->count) {
    if (rc > 1)
      return -1;
    glme_encoder_resize(enc, enc->buflen < 1024 ? enc->buflen : 1024);
    rc++;
    goto retry;
  }
  enc->count += n;
  return n;
}


int glme_encode_bytes(glme_encoder_t *enc, void *v, size_t vlen)
{
  int n, rc = 0;
  if (!enc)
    return 0;
 retry:
  n = gob_encode_uint64(&enc->buf[enc->count], enc->buflen - enc->count, (uint64_t)vlen);
  if (n > enc->buflen - enc->count) {
    if (rc > 1)
      return -1;
    glme_encoder_resize(enc, enc->buflen < 1024 ? enc->buflen : 1024);
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
    glme_encoder_resize(enc, vlen - (enc->buflen - enc->count));
  }
  memcpy(&enc->buf[enc->count], v, vlen);
  enc->count += vlen;
  return n+vlen;
}

// Local Variables:
// indent-tabs-mode: nil
// End:
