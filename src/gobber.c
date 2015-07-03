
// Copyright (c) Harri Rautila, 2015

#include <stdio.h>

// undefine __INLINE__ to allow redefinition
#ifdef __INLINE__
#undef __INLINE__
#endif

// define __INLINE__ to be empty
#define __INLINE__

#include "glme/gobber.h"

/*
 * Encodes unsigned 64bit integer into the specified buffer.
 *
 * Go gob package: "An unsigned integer is sent one of two
 * ways. If it is less than 128, it is sent as a byte with that value.
 * Otherwise it is sent as a minimal-length big-endian (high byte first) byte
 * stream holding the value, preceded by one byte holding the byte count,
 * negated. Thus 0 is transmitted as (00), 7 is transmitted as (07) and 256
 * is transmitted as (FE 01 00). "
 *
 * This implementation uses binary search to find highest non-zero byte of
 * the given unsigned number. Bytes are written into the result buffer
 * in reverse order from least signficant byte to most significant byte.
 * If the buffer "free" space is insufficient then no bytes are written into
 * the destination buffer. 
 *
 * @param buf
 *   The buffer into which to encode the parameter. The pointer must point
 *   to "empty" space in the buffer.
 * @param buf_size
 *   The number of bytes available.
 * @param uv
 *   The number to encode.
 *
 * @return 
 *   The number of bytes written by the encode operation. A return value greater
 *   than buf_size indicates error (buffer overflow) and no bytes are written.
 */
static inline
int __gob_encode_u64(char *buf, size_t buf_size, uint64_t ull)
{
  int k, nbytes;
  unsigned long long high;

  if (ull < 128) {
    if (buf_size > 0)
      *buf = (char)ull;
    return 1;
  }

  // binary search for length
  high = (ull >> 32);
  if ( high == 0 ) {
    // high bytes are zero; len <= 4
    if ( (ull >> 16) == 0 ) {
      nbytes =  (ull >> 8) == 0 ? 1 : 2;
    } else {
      nbytes = (ull >> 24) == 0 ? 3 : 4;
    }
  } else {
    // high bytes non-zero; len > 4
    if ( (high >> 16) == 0) {
      nbytes = (high >> 8) == 0 ? 5 : 6;
    } else {
      nbytes = (high >> 24) == 0 ? 7 : 8;
    }
  }
  
  if (nbytes >= buf_size) {
    // overflow; just return required space
    return nbytes + 1;
  }

  *buf++ = -nbytes;
  
  // copy in reverse order
  for (k = nbytes-1; k >= 0; k--) {
    buf[k] = ull & 0xFF;
    ull >>= 8;
  }
  return nbytes + 1;
}

/*
 * Decode unsigned intger from speficied buffer.
 *
 * @param ull
 *    Pointer to unsigned integer to receive decoded value.
 * @parm buf
 *    Source buffer
 * @param buf_size
 *    Number of bytes available in the buffer.
 *
 * @returns
 *    Encoded byte length. If larger that buf_size then error (under flow)
 *    occured.
 */
static inline
int __gob_decode_u64(uint64_t *ull, char *buf, size_t buf_size)
{
  int k, nbytes;
  uint64_t ulval = 0;

  *ull = 0;
  if (buf_size < 1)
    return 1;  // return 1 to indicate underflow, at least one byte needed

  if (*buf >= 0 && *buf < 128) {
    *ull = (uint64_t)*buf;
    return 1;
  }
  nbytes = -((char)*buf);
  buf_size--; buf++;
  for (k = nbytes; k > 0 && buf_size > 0; k--, buf_size--) {
    ulval = (ulval << 8) | (unsigned char)(*buf);
    buf++;
  }
  *ull = ulval;
  return nbytes+1;
}

/*
 * Flip given number to reverse byte order.
 */
static inline
uint64_t __gob_flip_u64(uint64_t ul)
{
  ul = ((ul >> 8)  & 0x00FF00FF00FF00FF) | ((ul & 0x00FF00FF00FF00FF) << 8);
  ul = ((ul >> 16) & 0x0000FFFF0000FFFF) | ((ul & 0x0000FFFF0000FFFF) << 16);
  ul =  (ul >> 32) | (ul << 32);
  return ul;
}


int gob_encode_uint64(char *buf, size_t buf_size, uint64_t uv)
{
  return __gob_encode_u64(buf, buf_size, uv);
}

int gob_encode_int64(char *buf, size_t buf_size, int64_t v)
{
  uint64_t u;
  if (v < 0) {
    u = (~v << 1) | 1;
  } else {
    u = (v << 1);
  }
  return __gob_encode_u64(buf, buf_size, u);
}


int gob_encode_bytes(char *buf, size_t buf_size, void *s, size_t len)
{
  int nbytes = gob_encode_uint64(buf, buf_size, len);
  if (nbytes >= buf_size) {
    return nbytes + len;
  }
  buf += nbytes;
  buf_size -= nbytes;
  if (len >= buf_size) {
    return nbytes + len;
  }
  memcpy(buf, s, len);
  return nbytes + len;
}

int gob_encode_double(char *buf, size_t buf_size, double v)
{
  uint64_t ul = __gob_flip_u64(*((uint64_t *)&v));
  return __gob_encode_u64(buf, buf_size, ul);
}

int gob_encode_type(char *buf, size_t buf_size, int id)
{
  return gob_encode_int64(buf, buf_size, (int64_t)id);
}

int gob_encode_start_struct(char *buf, size_t buf_size)
{
  return 0;
}

int gob_encode_end_struct(char *buf, size_t buf_size)
{
  if (buf_size > 0)
    *buf = '\0';
  return 1;
}



int gob_decode_uint64(uint64_t *ull, char *buf, size_t buf_size)
{
  int k, nbytes;
  uint64_t ulval = 0;
  *ull = 0;
  if (buf_size < 1)
    return 1;

  if (*buf >= 0 && *buf < 128) {
    *ull = (uint64_t)*buf;
    return 1;
  }
  nbytes = -1*((char)*buf);
  buf_size--; buf++;
  for (k = nbytes; k > 0 && buf_size > 0; k--, buf_size--) {
    ulval = (ulval << 8) | (unsigned char)(*buf);
    buf++;
  }
  *ull = ulval;
  return nbytes+1;
}

int gob_decode_int64(int64_t *v, char *buf, size_t buf_size)
{
  int n;
  uint64_t ull;
  n = gob_decode_uint64(&ull, buf, buf_size);
  if (n <= buf_size) {
    if (ull & 1) {
      *v = ~(ull >> 1);
    } else {
      *v = (ull >> 1);
    }
  }
  return n;
}


int gob_decode_double(double *v, char *buf, size_t buf_size)
{
  int n;
  uint64_t ull;
  n = gob_decode_uint64(&ull, buf, buf_size);
  if (n <= buf_size) {
    ull = __gob_flip_u64(ull);
    *v = *((double *)&ull);
  }
  return n;
}



/**
 */
int gob_decode_bytes(void *v, size_t vlen, char *buf, size_t buf_size)
{
  int n, nc;
  int64_t len;
  n = gob_decode_uint64(&len, buf, buf_size);
  if (n > buf_size) {
    return n;
  }
  nc = n + len;
  if (len > buf_size)
    len = buf_size;
  if (len < vlen)
    vlen = len;
  memcpy(v, &buf[n], vlen);
  return nc;
}


/**
 */
int gob_decode_string(char *s, size_t slen, char *buf, size_t buf_size)
{
  int n, nc;
  int64_t len;
  n = gob_decode_uint64(&len, buf, buf_size);
  if (n > buf_size) {
    return n;
  }
  nc = n + len;
  if (len > buf_size)
    len = buf_size;
  if (len < slen)
    slen = len;
  strncpy(s, &buf[n], slen);
  return nc;
}


// Local Variables:
// indent-tabs-mode: nil
// End:
