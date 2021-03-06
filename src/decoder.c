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
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "gobber.h"
#include "glme.h"

static inline
int __peek_base_type(glme_buf_t *dec, int id)
{
  unsigned int u = (id << 1);
  if (dec->current >= dec->count)
    return -1;
  return (dec->buf[dec->current] == (char)u) ? 1 : -1;
}

static inline
int __read_base_type(glme_buf_t *dec, int *id)
{
  if (dec->current >= dec->count)
    return -1;
  unsigned int u = dec->buf[dec->current];
  *id = u & 0x1 ? -((int)(u >> 1)) : (int)(u >> 1);
  return 1;
}

static inline
int __decode_base_type(glme_buf_t *dec, int id)
{
  unsigned int u = (id << 1);
  if (dec->current >= dec->count)
    return -1;
  return (dec->buf[dec->current++] == (char)u) ? 1 : -1;
}

// ----------------------------------------------------------------
// Base type value decoding functions

int glme_decode_value_uint64(glme_buf_t *dec, uint64_t *v)
{
  int n;

  n = gob_decode_uint64(v, &dec->buf[dec->current], dec->count-dec->current); 
  if (n < 0) {
    // under flow
    dec->last_error = GLME_E_UFLOW;
    return n;
  }
  dec->current += n;
  return n;
}

int glme_decode_peek_uint64(glme_buf_t *dec, uint64_t *v)
{
  int n;

  n = gob_decode_uint64(v, &dec->buf[dec->current], dec->count-dec->current); 
  if (n < 0) {
    // under flow
    dec->last_error = GLME_E_UFLOW;
    return n;
  }
  return n;
}

int glme_decode_value_int64(glme_buf_t *dec, int64_t *v)
{
  int n;
  n = gob_decode_int64(v, &dec->buf[dec->current], dec->count-dec->current); 
  if (n < 0) {
    // under flow
    dec->last_error = GLME_E_UFLOW;
    return -n;
  }
  dec->current += n;
  return n;
}

int glme_decode_peek_int64(glme_buf_t *dec, int64_t *v)
{
  int n;
  n = gob_decode_int64(v, &dec->buf[dec->current], dec->count-dec->current); 
  if (n < 0) {
    // under flow
    dec->last_error = GLME_E_UFLOW;
    return -n;
  }
  return n;
}

int glme_decode_value_uint32(glme_buf_t *dec, uint32_t *u)
{
  int n;
  uint64_t u64;
  n = glme_decode_value_uint64(dec, &u64);
  *u = n < 0 ? 0 : (uint32_t)u64;
  return n;
}

int glme_decode_value_int32(glme_buf_t *dec, int32_t *u)
{
  int n;
  int64_t u64;
  n = glme_decode_value_int64(dec, &u64);
  *u = n < 0 ? 0 : (int32_t)u64;
  return n;
}

int glme_decode_value_ulong(glme_buf_t *dec, unsigned long *u)
{
  int n;
  uint64_t u64;
  n = glme_decode_value_uint64(dec, &u64);
  *u = n < 0 ? 0 : (unsigned long)u64;
  return n;
}

int glme_decode_value_long(glme_buf_t *dec, long *d)
{
  int n;
  int64_t i64;
  n = glme_decode_value_int64(dec, &i64);
  *d = n < 0 ? 0 : (long)i64;
  return n;
}

int glme_decode_value_uint(glme_buf_t *dec, unsigned int *u)
{
  int n;
  uint64_t u64;
  n = glme_decode_value_uint64(dec, &u64);
  *u = n < 0 ? 0 : (unsigned int)u64;
  return n;
}

int glme_decode_value_int(glme_buf_t *dec, int *d)
{
  int n;
  int64_t i64;
  n = glme_decode_value_int64(dec, &i64);
  *d = n < 0 ? 0 : (int)i64;
  return n;
}

int glme_decode_value_double(glme_buf_t *dec, double *v)
{
  int n;
  n = gob_decode_double(v, &dec->buf[dec->current], dec->count-dec->current); 
  if (n < 0) {
    // under flow
    dec->last_error = GLME_E_UFLOW;
    return -n;
  }
  dec->current += n;
  return n;
}

int glme_decode_value_float(glme_buf_t *dec, float *v)
{
  int n;
  double dv;
  n = glme_decode_value_double(dec, &dv);
  *v = n < 0 ? 0.0 : (float)dv;
  return n;
}

int glme_decode_value_complex128(glme_buf_t *dec, double complex *v)
{
  int n;
  n = gob_decode_complex128(v, &dec->buf[dec->current], dec->count-dec->current); 
  if (n < 0) {
    // under flow
    dec->last_error = GLME_E_UFLOW;
    return -n;
  }
  dec->current += n;
  return n;
}

int glme_decode_value_complex64(glme_buf_t *dec, float complex *v)
{
  int n;
  double complex dv;
  n = glme_decode_value_complex128(dec, &dv);
  *v = n < 0 ? 0.0 : (float complex)dv;
  return n;
}

// ----------------------------------------------------------------
// Base type decoding functions

int glme_decode_uint64(glme_buf_t *dec, uint64_t *v)
{
  if (__decode_base_type(dec, GLME_UINT) < 0)
    return -1;
  int n = glme_decode_value_uint64(dec, v);
  return n < 0 ? n : n+1;
}

int glme_decode_int64(glme_buf_t *dec, int64_t *v)
{
  if (__decode_base_type(dec, GLME_INT) < 0)
    return -1;
  int n = glme_decode_value_int64(dec, v);
  return n < 0 ? n : n+1;
}

int glme_decode_uint32(glme_buf_t *dec, uint32_t *v)
{
  if (__decode_base_type(dec, GLME_UINT) < 0)
    return -1;
  int n = glme_decode_value_uint32(dec, v);
  return n < 0 ? n : n+1;
}

int glme_decode_int32(glme_buf_t *dec, int32_t *v)
{
  if (__decode_base_type(dec, GLME_INT) < 0)
    return -1;
  int n = glme_decode_value_int32(dec, v);
  return n < 0 ? n : n+1;
}

int glme_decode_ulong(glme_buf_t *dec, unsigned long *v)
{
  if (__decode_base_type(dec, GLME_UINT) < 0)
    return -1;
  int n = glme_decode_value_ulong(dec, v);
  return n < 0 ? n : n+1;
}

int glme_decode_long(glme_buf_t *dec, long *v)
{
  if (__decode_base_type(dec, GLME_INT) < 0)
    return -1;
  int n = glme_decode_value_long(dec, v);
  return n < 0 ? n : n+1;
}

int glme_decode_uint(glme_buf_t *dec, unsigned int *v)
{
  if (__decode_base_type(dec, GLME_UINT) < 0)
    return -1;
  int n = glme_decode_value_uint(dec, v);
  return n < 0 ? n : n+1;
}

int glme_decode_int(glme_buf_t *dec, int *v)
{
  if (__decode_base_type(dec, GLME_INT) < 0)
    return -1;
  int n = glme_decode_value_int(dec, v);
  return n < 0 ? n : n+1;
}

int glme_decode_double(glme_buf_t *dec, double *v)
{
  if (__decode_base_type(dec, GLME_FLOAT) < 0)
    return -1;
  int n = glme_decode_value_double(dec, v);
  return n < 0 ? n : n+1;
}

int glme_decode_float(glme_buf_t *dec, float *v)
{
  if (__decode_base_type(dec, GLME_FLOAT) < 0)
    return -1;
  int n = glme_decode_value_float(dec, v);
  return n < 0 ? n : n+1;
}

int glme_decode_complex128(glme_buf_t *dec, double complex *v)
{
  if (__decode_base_type(dec, GLME_COMPLEX) < 0)
    return -1;
  int n = glme_decode_value_complex128(dec, v);
  return n < 0 ? n : n+1;
}

int glme_decode_complex64(glme_buf_t *dec, float complex *v)
{
  if (__decode_base_type(dec, GLME_COMPLEX) < 0)
    return -1;
  int n = glme_decode_value_complex64(dec, v);
  return n < 0 ? n : n+1;
}

// ------------------------------------------------------------------
// byte array and string functions

int glme_decode_vector(glme_buf_t *dec, void *s, size_t len)
{
  int n;
  uint64_t dlen = 0;

  // we accept BYTE arrays and STRINGs
  if (__peek_base_type(dec, GLME_VECTOR) < 0 &&
      __peek_base_type(dec, GLME_STRING) < 0 ) {
    dec->last_error = GLME_E_TYPE;
    return -1;
  }

  dec->current++;

  n = gob_decode_uint64(&dlen, &dec->buf[dec->current], dec->count-dec->current);
  if (n < 0) {
    // underflow
    dec->last_error = GLME_E_UFLOW;
    return n;
  }
  if (dlen > dec->count - dec->current - n) {
    // underflow
    dec->last_error = GLME_E_UFLOW;
    return -(dlen+n);
  }
  // update current pointer;
  dec->current += n;
  memcpy(s, &dec->buf[dec->current], (len < dlen ? len : dlen));
  if (dlen < len)
    memset(&((char *)s)[dlen], 0, len-dlen);
  dec->current += dlen;
  return n + dlen + 1;
}

// Decodes byte array from buffer and allocate space for it.
int glme_decode_bytes(glme_buf_t *dec, void **s, size_t len)
{
  int n;
  uint64_t dlen = 0;
  char *nb;

  n = gob_decode_uint64(&dlen, &dec->buf[dec->current], dec->count-dec->current);
  if (n < 0) {
    // underflow
    dec->last_error = GLME_E_UFLOW;
    return n;
  }
  if (dlen > dec->count - dec->current - n) {
    // underflow
    dec->last_error = GLME_E_UFLOW;
    return -(dlen+n);
  }
  if (len == 0) {
    nb = glme_malloc(dec, dlen);
    if (nb) {
      memcpy(nb, &dec->buf[dec->current+n], dlen);
      *s = nb;
    }
  } else {
    nb = (char *)(*s);
    if (nb) {
      memcpy(nb, &dec->buf[dec->current+n], (dlen < len ? dlen : len));
      if (dlen < len)
        memset(&nb[dlen], 0, len-dlen);
    }
  }
  dec->current += n + dlen;
  return n + dlen;
}

int glme_decode_string(glme_buf_t *dec, char **s)
{
  int n;
  uint64_t dlen = 0;
  char *nb;

  if (__decode_base_type(dec, GLME_STRING) < 0)
    return -1;

  n = gob_decode_uint64(&dlen, &dec->buf[dec->current], dec->count-dec->current);
  if (n < 0) {
    // underflow
    dec->last_error = GLME_E_UFLOW;
    return n;
  }
  if (dlen > dec->count - dec->current - n) {
    // underflow
    dec->last_error = GLME_E_UFLOW;
    return -(dlen+n);
  }
  *s = (char *)0;
  nb = glme_malloc(dec, dlen+1);
  if (nb) {
    memcpy(nb, &dec->buf[dec->current+n], dlen);
    nb[dlen] = '\0';
    *s = nb;
  }
  return nb ? dlen+n+1 : -1;
}

// ----------------------------------------------------------------
// Type decoding functions

int glme_decode_type(glme_buf_t *dec, int *t)
{
  int n;
  int64_t i64 = 0;
  n = glme_decode_value_int64(dec, &i64);
  *t = (int)i64;
  return n;
}

int glme_decode_peek_type(glme_buf_t *dec, int *typeid)
{
  int n;
  int64_t i64;
  n = glme_decode_peek_int64(dec, &i64);
  if (n < 0)
    return n;
  *typeid = (int)i64;
  return n;
}

// ----------------------------------------------------------------
// Structure decoding functions


int glme_decode_delta_test(glme_buf_t *dec, unsigned int delta)
{
  int n;
  uint64_t offset;

  // read offset at read pointer
  n = gob_decode_uint64(&offset, &dec->buf[dec->current], dec->count - dec->current);
  if (n < 0)
    return n;

  return delta == offset ? 1 : 0;
}

int glme_decode_field(glme_buf_t *dec, unsigned int *delta, int etype, int flags, 
                      void *vptr, size_t *nlen, size_t esize, glme_decoder_f dfunc)
{
  int n, typeid;
  uint64_t offset, alen, __at_start = dec->current;
  void *nptr;

  // read offset at read pointer
  n = gob_decode_uint64(&offset, &dec->buf[dec->current], dec->count - dec->current);
  if (n < 0) {
    dec->last_error = GLME_E_UFLOW;
    return n;
  }

  if (offset == 0 || *delta == 0) {
    // end of struct or we have already seen end of struct
    *delta = 0;
    return 0;
  }

  if (*delta < offset) {
    // not yet
    *delta += 1;
    return 0;
  }
  // decode; set delta to 1, move read pointer and call glme_decoder_f function
  dec->current += n;
  if (glme_decode_peek_type(dec, &typeid) < 0)
    return -1;

  if (typeid == GLME_ARRAY && ((flags & GLME_F_ARRAY) == 0)) {
    // not expecting array
    dec->last_error = GLME_E_TYPE;
    return -1;
  }
  if (etype != 0 && typeid != GLME_ARRAY && typeid != etype) {
    // not this type
    dec->last_error = GLME_E_TYPE;
    return -1;
  }

  switch (typeid) {
  case GLME_ARRAY:
    if ((n = glme_decode_array_start(dec, &typeid, &alen)) < 0)
      return n;
    if (etype != 0 && typeid != etype) {
      dec->last_error = GLME_E_TYPE;
      return -1;
    }

    // assume that we need to allocate array element space
    nptr = (void *)0;
    if (*nlen > 0) {
      // if nlen > 0 expecting fixed size array; space provided
      nptr = (void *)(*((uint64_t **)vptr));
      // fixed size array must have enough space
      if (*nlen < alen)
        return -1;
    }
    if ((n = glme_decode_array_data(dec, &nptr, alen, esize, dfunc)) < 0)
      return n;
    if (*nlen == 0) {
      // return allocated pointer and actual length;
      *nlen = alen;
      *((uint64_t **)vptr) = nptr;
    }
    break;

  case GLME_STRING:
    // variable string
    n = glme_decode_string(dec, (char **)&nptr);
    if (n < 0)
      return -n;
    *((uint64_t **)vptr) = nptr;
    break;

  case GLME_VECTOR:
    // fixed size byte vector; esize is 1 and nlen is number of bytes
    n = glme_decode_vector(dec, vptr, *nlen);
    break;

  case GLME_INT:
  case GLME_UINT:
  case GLME_FLOAT:
  case GLME_COMPLEX:
    if (!dfunc) {
      dec->last_error = GLME_E_NODEC;
      return -1;
    }
    n = (*dfunc)(dec, vptr);
    break;

  default:
    if ((n = glme_decode_type(dec, &typeid)) < 0)
      return n;
    if (etype != 0 && etype != typeid) {
      dec->last_error = GLME_E_TYPE;
      return -1;
    }
    // embedded structure  or structure pointer
    if (flags & GLME_F_PTR) {
      // vptr is pointer to pointer (void **);
      if (esize == 0) {
        if ((esize = glme_get_typesize(dec, typeid)) == 0) {
          dec->last_error = GLME_E_NOSIZE;
          return -1;
        }
      }
      nptr = glme_malloc(dec, esize);
      if (! nptr) {
        dec->last_error = GLME_E_NOMEM;
        return -1;
      }
    } else {
      nptr = vptr;
    }
    if (!dfunc) {
      dfunc = glme_get_decoder(dec, typeid);
      if (!dfunc) {
        dec->last_error = GLME_E_NODEC;
        return -1;
      }
    }
    if ((n = (*dfunc)(dec, nptr)) < 0) {
      // if we have allocated memory, release it.
      if (flags & GLME_F_PTR)
        free(nptr);
      return n;
    }
    if (flags & GLME_F_PTR) {
      // it was a struct pointer needing 
      // return pointer to the memory block
      *((uintptr_t **)vptr) = nptr;
    }
    break;
  }
  if (n > 0)
    *delta = 1;
  return n < 0 ? n : dec->current - __at_start;
}


int glme_decode_struct(glme_buf_t *dec, int typeid, void **dptr, size_t esize, glme_decoder_f dfunc)
{
  uint64_t __at_start = dec->current;
  int n, typ;
  void *nptr, *sptr = (*dptr);

  if (! dfunc)
    return -1;

  if (glme_decode_type(dec, &typ) < 0)
    return -1;

  // if specified type id is non-zero check decoded id.
  if (typeid != 0 && typ != typeid) {
    dec->last_error = GLME_E_TYPE;
    return -1;
  }

  if (!dfunc) {
    // try to find decoder function
    if (!(dfunc = glme_get_decoder(dec, typeid))) {
      dec->last_error = GLME_E_NODEC;
      return -1;
    }
  }

  nptr = sptr;
  if (!sptr) {
    // allocate space
    if (esize == 0 && (esize = glme_get_typesize(dec, typ)) == 0) {
      dec->last_error = GLME_E_NOSIZE;
      return -1;
    }
    nptr = glme_malloc(dec, esize);
    if (!nptr) {
      dec->last_error = GLME_E_NOMEM;
      return -1;
    }
  }
  if ((n = (*dfunc)(dec, nptr) ) < 0) {
    if (!sptr)
      glme_free(dec, nptr);
    return n;
  }
  if (!sptr)
    *dptr = nptr;
  return dec->current - __at_start;
}

int glme_decode_value_struct(glme_buf_t *dec, void **dptr, size_t esize, glme_decoder_f dfunc)
{
  uint64_t __at_start = dec->current;
  int n;
  void *nptr, *sptr = (*dptr);

  if (!dfunc) {
    dec->last_error = GLME_E_NODEC;
    return -1;
  }

  nptr = sptr;
  if (!sptr) {
    if (esize == 0) {
      dec->last_error = GLME_E_NOSIZE;
      return -1;
    }
    nptr = glme_malloc(dec, esize);
    if (!nptr) {
      dec->last_error = GLME_E_NOMEM;
      return -1;
    }
  }
  // decode
  if ((n = (*dfunc)(dec, nptr) ) < 0) {
    if (!sptr)
      glme_free(dec, nptr);
    return n;
  }
  if (!sptr)
    *dptr = nptr;
  return dec->current - __at_start;
}


int glme_decode_start_struct(glme_buf_t *dec, int *delta)
{
  if (!delta)
    return -1;
  *delta = 1;
  return 0;
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

// ---------------------------------------------------------------------
// Array decoding functions

int glme_decode_array_start(glme_buf_t *dec, int *typeid, size_t *len)
{
  int n, nc = 0;
  n = __decode_base_type(dec, GLME_ARRAY);
  if (n < 0)
    return n;
  nc += n;
  n = glme_decode_type(dec, typeid);
  if (n < 0)
    return n;
  nc += n;
  n = glme_decode_value_uint64(dec, len);
  return n < 0 ? n : nc + n;
}

int glme_decode_array_header(glme_buf_t *dec, int *typeid, size_t *len)
{
  int n0, n1;
  if ((n0 = glme_decode_type(dec, typeid)) < 0)
    return -1;
  if ((n1 = glme_decode_value_uint64(dec, len)) < 0)
    return -1;
  return n1 + n0;
}

int glme_decode_array_data(glme_buf_t *dec, void **dst,
                           size_t len, size_t esize, glme_decoder_f func)
{
  char *ptr = (char *)(*dst);
  int k, n;
  size_t i, __at_start = dec->current;

  if (len == 0)
    return 0;

  if (! ptr) {
    ptr = (char *)glme_calloc(dec, len, esize);
    if (! ptr)
      return -1; 
    *(char **)dst = ptr;
  }
  for (k = 0, i = 0; k < len; k++, i += esize) {
    if ((n = (*func)(dec, (void *)&ptr[i])) < 0)
      return n;
  }
  return dec->current - __at_start;
}

// decode full array
int glme_decode_array(glme_buf_t *dec, int *typeid, void **dst,
                      size_t *len, size_t esize, glme_decoder_f dfunc)
{
  size_t alen;
  uint64_t __at_start = dec->current;

  if (__decode_base_type(dec, GLME_ARRAY) < 0)
    return -1;
  
  if (glme_decode_array_header(dec, typeid, &alen) < 0)
    return -1;

  if (*len > 0 && *len < alen)
    return -1;
  
  if (glme_decode_array_data(dec, dst, alen, esize, dfunc) < 0)
    return -1;
  
  return dec->current - __at_start;
}


// decode array value (Array sans ARRAY typeid)
int glme_decode_value_array(glme_buf_t *dec, int *typeid, void **dst,
                            size_t *len, size_t esize, glme_decoder_f dfunc)
{
  size_t alen;
  uint64_t __at_start = dec->current;

  if (glme_decode_array_header(dec, typeid, &alen) < 0)
    return -1;

  if (*len > 0 && *len < alen)
    return -1;
  
  if (glme_decode_array_data(dec, dst, alen, esize, dfunc) < 0)
    return -1;
  
  return dec->current - __at_start;
}


// Local Variables:
// indent-tabs-mode: nil
// End:
