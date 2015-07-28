/*
 * Copyright (c)  Harri Rautila, 2015
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 *    * Redistributions of source code must retain the above copyright notice, 
 *      this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *    * Neither the name of the Authors nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* This file is part of https://github.com/hrautila/glme repository. */

#include <stdio.h>
#if defined(__x86_64__)
#include <x86intrin.h>
#endif

#include "gobber.h"

// 64bit floating point vs. unsigned 64bit int
union _u {
  uint64_t ul;
  double ud;
};

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
 *   The number of bytes written by the encode operation. A negative value
 *   indicate buffer overflow and number of bytes needed. In case of error
 *   no bytes are written.
 */
static inline
int __gob_encode_u64(char *buf, size_t buf_size, uint64_t ull)
{
  int nbytes;

  if (ull < 128) {
    if (buf_size < 1) 
      return -1;
    *buf = (char)ull;
    return 1;
  }

#if defined(__x86_64__)
  // ull is not zero here; result of BSR is undefined if ull is zero
  nbytes = (__bsrq(ull) >> 3) + 1;
#else  
  // binary search for length
  unsigned long long high;
  high = (ull >> 32);
  if ( high == 0 ) {
    // high bytes are zero; len <= 4
    if ( (ull >> 16) == 0 ) {
      nbytes = (ull >> 8) == 0 ? 1 : 2;
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
#endif
  
  if (nbytes >= buf_size) {
    // overflow; just return required space
    return -(nbytes + 1);
  }

  *buf++ = -nbytes;
  
  // without loop in reverse order; register to memory write
  switch (nbytes) {
  case 8:
    buf[7] = ull & 0xFF;
    ull >>= 8;
  case 7:
    buf[6] = ull & 0xFF;
    ull >>= 8;
  case 6:
    buf[5] = ull & 0xFF;
    ull >>= 8;
  case 5:
    buf[4] = ull & 0xFF;
    ull >>= 8;
  case 4:
    buf[3] = ull & 0xFF;
    ull >>= 8;
  case 3:
    buf[2] = ull & 0xFF;
    ull >>= 8;
  case 2:
    buf[1] = ull & 0xFF;
    ull >>= 8;
  case 1:
    buf[0] = ull & 0xFF;
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
 *    Encoded byte length. If negative the buffer underflow occured.
 */
static inline
int __gob_decode_u64(uint64_t *ull, char *buf, size_t buf_size)
{
  int nbytes;
  uint64_t ulval = 0;

  *ull = 0;
  if (buf_size < 1)
    return -1;  // return 1 to indicate underflow, at least one byte needed

  if (*buf >= 0 && *buf < 128) {
    *ull = (uint64_t)*buf;
    return 1;
  }
  nbytes = -((char)*buf);
  if (nbytes >= buf_size)
    return -(nbytes+1);

  buf_size--; buf++;
  switch (nbytes) {
  case 8:
    ulval = (ulval << 8) | (unsigned char)(*buf++);
  case 7:
    ulval = (ulval << 8) | (unsigned char)(*buf++);
  case 6:
    ulval = (ulval << 8) | (unsigned char)(*buf++);
  case 5:
    ulval = (ulval << 8) | (unsigned char)(*buf++);
  case 4:
    ulval = (ulval << 8) | (unsigned char)(*buf++);
  case 3:
    ulval = (ulval << 8) | (unsigned char)(*buf++);
  case 2:
    ulval = (ulval << 8) | (unsigned char)(*buf++);
  case 1:
    ulval = (ulval << 8) | (unsigned char)(*buf);
  }
#if 0
  // old code here
  for (k = nbytes; k > 0 && buf_size > 0; k--, buf_size--) {
    ulval = (ulval << 8) | (unsigned char)(*buf);
    buf++;
  }
#endif
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
  if (nbytes < 0) {
    return nbytes - len;
  }
  buf += nbytes;
  buf_size -= nbytes;
  if (len >= buf_size) {
    return -(nbytes + len);
  }
  memcpy(buf, s, len);
  return nbytes + len;
}


int gob_encode_double(char *buf, size_t buf_size, double v)
{
  union _u uu = { .ud = v };
  uu.ul = __gob_flip_u64(uu.ul);
  return __gob_encode_u64(buf, buf_size, uu.ul);
}

int gob_encode_complex128(char *buf, size_t buf_size, double complex v)
{
  int n1, n0;
  if ((n0 = gob_encode_double(buf, buf_size, creal(v))) < 0)
    return n0;
  if ((n1 = gob_encode_double(&buf[n0], buf_size-n0, cimag(v))) < 0)
    return n1;
  return n1+n0;
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
  return __gob_decode_u64(ull, buf, buf_size);
}

int gob_decode_int64(int64_t *v, char *buf, size_t buf_size)
{
  int n;
  uint64_t ull;
  n = __gob_decode_u64(&ull, buf, buf_size);
  if (n > 0) {
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
  union _u uu;
  n = __gob_decode_u64(&uu.ul, buf, buf_size);
  if (n > 0) {
    uu.ul = __gob_flip_u64(uu.ul);
    *v = uu.ud;
  }
  return n;
}

int gob_decode_complex128(double complex *v, char *buf, size_t buf_size)
{
  double re, im;
  int n0, n1;
  if ((n0 = gob_decode_double(&re, buf, buf_size)) < 0)
    return n0;
  if ((n1 = gob_decode_double(&im, &buf[n0], buf_size-n0)) < 0)
    return n1;
  *v = re + im*I;
  return n0 + n1;
}

/**
 */
int gob_decode_bytes(void *v, size_t vlen, char *buf, size_t buf_size)
{
  int n, nc;
  uint64_t len;
  n = __gob_decode_u64(&len, buf, buf_size);
  if (n < 0) {
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
  uint64_t len;
  n = __gob_decode_u64(&len, buf, buf_size);
  if (n < 0) {
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

int gob_encode_ulong(char *buf, size_t buf_size, unsigned long v)
{
  return gob_encode_uint64(buf, buf_size, (uint64_t)v);
}

int gob_encode_long(char *buf, size_t buf_size, long v)
{
  return gob_encode_int64(buf, buf_size, (int64_t)v);
}

int gob_encode_uint(char *buf, size_t buf_size, unsigned int v)
{
  return gob_encode_uint64(buf, buf_size, (uint64_t)v);
}

int gob_encode_int(char *buf, size_t buf_size, int v)
{
  return gob_encode_int64(buf, buf_size, (int64_t)v);
}

int gob_encode_bool(char *buf, size_t buf_size, int v)
{
  return gob_encode_uint64(buf, buf_size, (int64_t)(v != 0));
}

int gob_encode_float(char *buf, size_t buf_size, float v)
{
  return gob_encode_double(buf, buf_size, (double)v);
}

int gob_encode_complex64(char *buf, size_t buf_size, float complex v)
{
  return gob_encode_complex128(buf, buf_size, (double complex)v);
}


int gob_encode_string(char *buf, size_t buf_size, char *s)
{
  return gob_encode_bytes(buf, buf_size, s, strlen(s)+1);
}



int gob_decode_ulong(unsigned long *v, char *buf, size_t buf_size)
{
  int n;
  uint64_t vv = 0;
  n = gob_decode_uint64(&vv, buf, buf_size);
  *v = (unsigned long)vv;
  return n;
}

int gob_decode_long(long *v, char *buf, size_t buf_size)
{
  int n;
  int64_t vv = 0;
  n = gob_decode_int64(&vv, buf, buf_size);
  *v = (long)vv;
  return n;
}

int gob_decode_uint(unsigned int *v, char *buf, size_t buf_size)
{
  int n;
  uint64_t vv = 0;
  n = gob_decode_uint64(&vv, buf, buf_size);
  *v = (unsigned int)vv;
  return n;
}

int gob_decode_int(int *v, char *buf, size_t buf_size)
{
  int n;
  int64_t vv = 0;
  n = gob_decode_int64(&vv, buf, buf_size);
  *v = (int)vv;
  return n;
}

int gob_decode_float(float *v, char *buf, size_t buf_size)
{
  int n;
  double vv = 0.0;
  n = gob_decode_double(&vv, buf, buf_size);
  *v = (float)vv;
  return n;
}

int gob_decode_complex64(float complex *v, char *buf, size_t buf_size)
{
  int n;
  double complex vv = 0.0 + 0.0*I;
  n = gob_decode_complex128(&vv, buf, buf_size);
  *v = (float complex)vv;
  return n;
}


// Local Variables:
// indent-tabs-mode: nil
// End:
