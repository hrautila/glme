
// Copyright (c) Harri Rautila, 2015

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "gobber.h"
#include "glme.h"


int glme_decode_uint64(glme_buf_t *dec, uint64_t *v)
{
  int n;

  n = gob_decode_uint64(v, &dec->buf[dec->current], dec->count-dec->current); 
  if (n > dec->count - dec->current) {
    // under flow
    return -n;
  }
  dec->current += n;
  return n;
}

int glme_decode_uint64_peek(glme_buf_t *dec, uint64_t *v)
{
  int n;

  n = gob_decode_uint64(v, &dec->buf[dec->current], dec->count-dec->current); 
  if (n > dec->count - dec->current) {
    // under flow
    return -n;
  }
  return n;
}

int glme_decode_int64(glme_buf_t *dec, int64_t *v)
{
  int n;
  n = gob_decode_int64(v, &dec->buf[dec->current], dec->count-dec->current); 
  if (n > dec->count - dec->current) {
    // under flow
    return -n;
  }
  dec->current += n;
  return n;
}

int glme_decode_int64_peek(glme_buf_t *dec, int64_t *v)
{
  int n;
  n = gob_decode_int64(v, &dec->buf[dec->current], dec->count-dec->current); 
  if (n > dec->count - dec->current) {
    // under flow
    return -n;
  }
  return n;
}

int glme_decode_ulong(glme_buf_t *dec, unsigned long *u)
{
  int n;
  uint64_t u64;
  n = glme_decode_uint64(dec, &u64);
  *u = n < 0 ? 0 : (unsigned long)u64;
  return n;
}

int glme_decode_long(glme_buf_t *dec, long *d)
{
  int n;
  int64_t i64;
  n = glme_decode_int64(dec, &i64);
  *d = n < 0 ? 0 : (long)i64;
  return n;
}

int glme_decode_uint(glme_buf_t *dec, unsigned int *u)
{
  int n;
  uint64_t u64;
  n = glme_decode_uint64(dec, &u64);
  *u = n < 0 ? 0 : (unsigned int)u64;
  return n;
}

int glme_decode_int(glme_buf_t *dec, int *d)
{
  int n;
  int64_t i64;
  n = glme_decode_int64(dec, &i64);
  *d = n < 0 ? 0 : (int)i64;
  return n;
}

int glme_decode_double(glme_buf_t *dec, double *v)
{
  int n;
  n = gob_decode_double(v, &dec->buf[dec->current], dec->count-dec->current); 
  if (n > dec->count - dec->current) {
    // under flow
    return -n;
  }
  dec->current += n;
  return n;
}

int glme_decode_float(glme_buf_t *dec, float *v)
{
  int n;
  double dv;
  n = glme_decode_double(dec, &dv);
  *v = n < 0 ? 0.0 : (float)dv;
  return n;
}

int glme_decode_vec(glme_buf_t *dec, void *s, size_t len)
{
  int n;
  int64_t dlen = 0;
  n = gob_decode_uint64(&dlen, &dec->buf[dec->current], dec->count-dec->current);
  if (n > dec->count - dec->current) {
    // underflow
    return -n;
  }
  if (dlen > dec->count - dec->current - n) {
    // underflow
    return -(dlen+n);
  }
  // update current pointer;
  dec->current += n;
  memcpy(s, &dec->buf[dec->current], len);
  dec->current += dlen;
  return n + dlen;
}

int glme_decode_bytes(glme_buf_t *dec, void **s)
{
  int n;
  int64_t dlen = 0;
  char *nb;
  n = gob_decode_uint64(&dlen, &dec->buf[dec->current], dec->count-dec->current);
  if (n > dec->count - dec->current) {
    // underflow
    return -n;
  }
  if (dlen > dec->count - dec->current - n) {
    // underflow
    return -(dlen+n);
  }
  nb = malloc(dlen);
  if (nb) {
    memcpy(nb, &dec->buf[dec->current+n], dlen);
    *s = nb;
  }
  dec->current += n + dlen;
  return n + dlen;
}

int glme_decode_string(glme_buf_t *dec, char **s)
{
  return glme_decode_bytes(dec, (void **)s);
}

int glme_decode_end_struct(glme_buf_t *dec)
{
  int n;
  uint64_t endm;
  n = gob_decode_uint64(&endm, &dec->buf[dec->current], dec->count-dec->current);
  if (n != 1) {
    return -1;
  }
  if (endm != 0) {
    return -1;
  }
  dec->current += n;
  return n;
}

// Local Variables:
// indent-tabs-mode: nil
// End:
