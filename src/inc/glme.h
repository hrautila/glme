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

// GLME is Gob Like Message Encoding

#ifndef __GLME_H
#define __GLME_H

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <complex.h>

// for inline base functions (see glme.c)
#ifndef __GLME_INLINE__
#define __GLME_INLINE__ extern inline
#endif


#ifdef __cplusplus
extern "C" {
#endif

  enum glme_types {
    GLME_ANY            = 0,
    GLME_BOOLEAN        = 1,
    GLME_INT            = 2,
    GLME_UINT           = 3,
    GLME_FLOAT          = 4,
    GLME_VECTOR         = 5,
    GLME_STRING         = 6,
    GLME_COMPLEX        = 7,
    GLME_ARRAY          = 10,
    GLME_MAP            = 11, /* Reserved */
    GLME_NAMED_STRUCT   = 12, /* Reserved */
    GLME_NAMED_MAP      = 13, /* Reserved */
    GLME_BASE_MAX       = 15, /* */
    GLME_USER_MIN       = 16  /* first user available type id */
  };

  enum glme_flags {
    GLME_F_NONE   = 0x0,
    GLME_F_ARRAY  = 0x1,
    GLME_F_PTR    = 0x2
  };

  /* Not yet used, needs some thought. */
  enum glme_errors {
    GLME_E_INPUT  = -1,
    GLME_E_INVAL  = -2,
    GLME_E_TYPE   = -3,
    GLME_E_NOENC  = -4,
    GLME_E_NODEC  = -5,
    GLME_E_NOSIZE = -6,
    GLME_E_NOMEM  = -7,
    GLME_E_UFLOW  = -8,
    GLME_E_OFLOW  = -9
  };

// forward spec
typedef struct glme_base_s glme_base_t;

/**
 * Gob Like Message Encoding buffer
 */
typedef struct glme_buf_s {
  char *buf;            //   Data buffer
  size_t buflen;	//   Size of buffer in bytes
  size_t count;		//   Number of bytes writen into the buffer (count <= buflen)
  size_t current;	//   Current read pointer (current <= count <= buflen)
  int owner;            //   Buffer owner flag
  void *user;           ///< User context for encoding/decoding functions.
  glme_base_t *base;    ///< Encoder/decoder registry
  int last_error;       ///< Last error 
} glme_buf_t;


typedef int (*glme_encoder_f)(glme_buf_t *, const void *);
typedef int (*glme_decoder_f)(glme_buf_t *, void *);

typedef struct glme_spec_s
{
  int typeid;
  size_t size;         // sizeof(<struct message>)
  glme_encoder_f encoder;
  glme_decoder_f decoder;
} glme_spec_t;

/**
 * User defined memory allocation functions.
 */
typedef struct glme_allocator_s
{
  void *(*malloc)(size_t);              ///< Allocate memory
  void (*free)(void *);                 ///< Release memory function
  void *(*realloc)(void *, size_t);     ///< Reallocation
  void *(*calloc)(size_t, size_t);      ///< Allocation in blocks
} glme_allocator_t;

/**
 * Handler base
 */
struct glme_base_s
{
  void *(*malloc)(size_t);
  void (*free)(void *);
  void *(*realloc)(void *, size_t);
  void *(*calloc)(size_t, size_t);
  unsigned int nelem;
  glme_spec_t *handlers;
  int owner;
};


// ---------------------------------------------------------------------
// Type specific encoder/decoder function spec.

/**
 * Initialize type encoder/decoder specification.
 */
__GLME_INLINE__
glme_spec_t *glme_spec_init(glme_spec_t *spec, int typeid,
                            glme_encoder_f encoder, glme_decoder_f decoder, size_t size)
{
  *spec = (glme_spec_t){typeid, size, encoder, decoder};
  return spec;
}

// ---------------------------------------------------------------------
// Handler base for registered typeids.


/**
 * Initialize handler base.
 *
 * @param base
 *   Handler base.
 * @param specs
 *   Array of encoder/decoder specs. May be null.
 * @param nlem
 *   Number of elements in handler array. If spec is null then space will allocated
 *   for nelem entries.
 * @param alloc
 *   Optional memory allocator functions. If not provided standard system functios
 *   are used.
 */
extern void glme_base_init(glme_base_t *base, glme_spec_t *specs, unsigned int nelem,
                           glme_allocator_t *alloc);

extern glme_spec_t *glme_base_find(glme_base_t *base, int typeid);

/**
 * Register typeid handlers.
 */
  extern int glme_base_register(glme_base_t *base, glme_spec_t *spec);
/**
 * Unregister typeid handlers.
 */
extern void glme_base_unregister(glme_base_t *base, int typeid);

/**
 * Release handler table if allocated at initialization.
 */
__GLME_INLINE__
void glme_base_release(glme_base_t *base)
{
  if (base && base->owner) 
    free(base->handlers);
}


/**
 * Get spec for typeid.
 */
__GLME_INLINE__
glme_spec_t *glme_get_spec(glme_buf_t *gb, int typeid)
{
  return glme_base_find(gb->base, typeid);
}

/**
 * Get element size for typeid.
 */
__GLME_INLINE__
size_t glme_get_typesize(glme_buf_t *gb, int typeid)
{
  glme_spec_t *s = glme_base_find(gb->base, typeid);
  return s ? s->size : 0;
}

/**
 * Find encoder function for typeid.
 */
__GLME_INLINE__
glme_encoder_f glme_get_encoder(glme_buf_t *gb, int typeid)
{
  glme_spec_t *s = glme_base_find(gb->base, typeid);
  return s ? s->encoder : (glme_encoder_f)0;
}

/**
 * Find decoder function for typeid.
 */
__GLME_INLINE__
glme_decoder_f glme_get_decoder(glme_buf_t *gb, int typeid)
{
  glme_spec_t *s = glme_base_find(gb->base, typeid);
  return s ? s->decoder : (glme_decoder_f)0;
}

/**
 * Allocate memory nbyt bytes of memory.
 */
__GLME_INLINE__
void *glme_malloc(glme_buf_t *gb, size_t nbyt)
{
  return gb->base && gb->base->malloc
    ? (*gb->base->malloc)(nbyt)
    : malloc(nbyt);
}

/**
 * Reallocate memory pointed by ptr to new size of nbyt bytes.
 */
__GLME_INLINE__
void *glme_realloc(glme_buf_t *gb, void *ptr, size_t nbyt)
{
  return gb->base && gb->base->realloc
    ? (*gb->base->realloc)(ptr, nbyt)
    : realloc(ptr, nbyt);
}

/**
 * Allocate memory block of size nelem*nbyt bytes.
 */
__GLME_INLINE__
void *glme_calloc(glme_buf_t *gb, size_t nelem, size_t nbyt)
{
  return gb->base && gb->base->calloc
    ? (*gb->base->calloc)(nelem, nbyt)
    : calloc(nelem, nbyt);
}

/**
 * Free memory ptr points to.
 */
__GLME_INLINE__
void glme_free(glme_buf_t *gb, void *ptr)
{
  if (gb->base && gb->base->free)
    (*gb->base->free)(ptr);
  else
    free(ptr);
}

/**
 * Allocate space for typeid object.
 */
__GLME_INLINE__
void *glme_type_new(glme_buf_t *gb, int typeid)
{
  glme_spec_t *s = glme_get_spec(gb, typeid);
  if (!s || s->size == 0)
    return (void *)0;
  return glme_malloc(gb, s->size);
}


// ---------------------------------------------------------------------
// GLME base functions.



/**
 * Initialize the specified gbuf with space of len bytes.
 *
 * @param gbuf
 *   The buffer.
 * @param len
 *   Requested initial buffer space in bytes.
 *
 * @return
 *   Initialized buffer.
 */
__GLME_INLINE__
glme_buf_t *glme_buf_init(glme_buf_t *gbuf, size_t len)
{
  if (gbuf) {
    gbuf->count = gbuf->current = 0;
    gbuf->buflen = gbuf->owner = 0;
    // allow initialization to zero size
    if (len > 0) {
      gbuf->buf = malloc(len);
      if (! gbuf->buf)
	return (glme_buf_t *)0;
      gbuf->owner = 1;
    } else {
      gbuf->buf = (char *)0;
    }
    gbuf->buflen = gbuf->buf ? len : 0;
    gbuf->user = (void *)0;
    gbuf->base = (glme_base_t *)0;
    gbuf->last_error = 0;
  }
  return gbuf;
}

/**
 * Make glme_buf from spesified data buffer.
 *
 * @param gbuf
 *   The buffer
 * @param data
 *   Data buffer
 * @param len
 *   Length of the buffer
 * @param count
 *   Length of the encoded content in the data buffer
 * @return
 *   Initialized glme_buf.
 */
__GLME_INLINE__
glme_buf_t *glme_buf_make(glme_buf_t *gbuf, char *data, size_t len, size_t count)
{
  gbuf->buf = data;
  gbuf->buflen = len;
  gbuf->count = count;
  gbuf->current = 0;
  gbuf->owner = 0;
  return gbuf;
}

/**
 * Close the glme_buf. Releases allocated buffer and reset read pointers.
 */
__GLME_INLINE__
void glme_buf_close(glme_buf_t *gbuf)
{
  if (gbuf) {
    if (gbuf->buf && gbuf->owner)
      glme_free(gbuf, gbuf->buf);
    gbuf->buf = (char *)0;
    gbuf->buflen = 0;
    gbuf->count = 0;
    gbuf->current = 0;
  }
}

/**
 * Reset glme_buf read pointer.
 */
__GLME_INLINE__
void glme_buf_reset(glme_buf_t *gbuf)
{
  if (gbuf)
    gbuf->current = 0;
}

/**
 * Get glme_buf read pointer.
 */
__GLME_INLINE__
size_t glme_buf_at(glme_buf_t *gbuf)
{
  return gbuf ? gbuf->current : 0;
}

/**
 * Set glme_buf read pointer.
 */
__GLME_INLINE__
void glme_buf_seek(glme_buf_t *gbuf, size_t pos)
{
  if (gbuf)  
    gbuf->current = pos < gbuf->count ? pos : gbuf->count;
}

/**
 * Pushback read pointer.
 */
extern void glme_buf_pushback(glme_buf_t *gbuf, size_t n);

/**
 * Clear glme_buf contents.
 */
__GLME_INLINE__
void glme_buf_clear(glme_buf_t *gbuf)
{
  if (gbuf) {
    gbuf->count = gbuf->current = 0;
  }
}

/**
 * Get content
 */
__GLME_INLINE__
char *glme_buf_data(glme_buf_t *gbuf)
{
  return gbuf ? gbuf->buf : (char *)0;
}

/**
 * Get encoded content length.
 */
__GLME_INLINE__
size_t glme_buf_len(glme_buf_t *gbuf)
{
  return gbuf ? gbuf->count : 0;
}

/**
 * Get size.
 */
__GLME_INLINE__
size_t glme_buf_size(glme_buf_t *gbuf)
{
  return gbuf ? gbuf->buflen : 0;
}

/**
 * Disown glme_buf allocated space.
 */
__GLME_INLINE__
void glme_buf_disown(glme_buf_t *gbuf)
{
  if (gbuf)
    gbuf->owner = 0;
}

/**
 * Own glme_buf allocated space.
 */
__GLME_INLINE__
void glme_buf_own(glme_buf_t *gbuf)
{
  if (gbuf)
    gbuf->owner = 1;
}



/**
 * Read length prefix message from file descriptor to spesified buffer.
 *
 * @return
 *    total number of bytes read.
 */
extern int glme_buf_readm(glme_buf_t *gbuf, int fd, size_t maxlen);

/**
 * Write encoded content to file descriptor as length prefix message.
 */
extern int glme_buf_writem(glme_buf_t *gbuf, int fd);


/**
 * Increase size of the glme_buf.
 *
 * @param gbuf
 *   The glme_buf
 * @parm increase
 *   Number of bytes to increase the buffer
 *
 * Resizing a glme_buf with externally allocated buffer may cause memory leaks
 * if old buffer is not properly released elsewhere.
 *
 * @return
 *   New size of the internal buffer or zero on reallocation failure.
 */
extern size_t glme_buf_resize(glme_buf_t *gbuf, size_t increase);



/**
 * Encode unsigned 64 bit integer type or value into the specified buffer.
 *
 * @param gbuf
 *    Buffer to write data into.
 * @param v
 *    Data to encode
 *
 * @returns
 *    Number of bytes writen. If buffer buffer resize fails returns -1 for
 *    error.
 */
extern int glme_encode_uint64(glme_buf_t *gbuf, const uint64_t *v);
extern int glme_encode_value_uint64(glme_buf_t *gbuf, const uint64_t *v);

/**
 * Encode signed 64 bit integer type or value into the specified buffer.
 *
 * @see glme_encode_uint64
 */
extern int glme_encode_int64(glme_buf_t *gbuf, const int64_t *v);
extern int glme_encode_value_int64(glme_buf_t *gbuf, const int64_t *v);

/**
 * Encode double precision floating point number into the specified buffer.
 *
 * @see glme_encode_uint64
 */
extern int glme_encode_double(glme_buf_t *gbuf, const double *v);
extern int glme_encode_value_double(glme_buf_t *gbuf, const double *v);

/**
 * Encode double precision complex number into the specified buffer.
 *
 * @see glme_encode_uint64
 */
extern int glme_encode_complex128(glme_buf_t *gbuf, const double complex *v);
extern int glme_encode_value_complex128(glme_buf_t *gbuf, const double complex *v);


/**
 * Encode unsigned long type or value into the specified buffer.
 *
 * @see glme_encode_uint64
 */
extern int glme_encode_ulong(glme_buf_t *gbuf, const unsigned long *v);
extern int glme_encode_value_ulong(glme_buf_t *gbuf, const unsigned long *v);

/**
 * Encode signed long  type or value into the specified buffer.
 *
 * @see glme_encode_uint64
 */
extern int glme_encode_long(glme_buf_t *gbuf, const long *v);
extern int glme_encode_value_long(glme_buf_t *gbuf, const long *v);

/**
 * Encode unsigned int type or value into the specified buffer.
 *
 * @see glme_encode_uint64
 */
extern int glme_encode_uint(glme_buf_t *gbuf, const unsigned int *v);
extern int glme_encode_value_uint(glme_buf_t *gbuf, const unsigned int *v);

/**
 * Encode signed int type or value into the specified buffer.
 *
 * @see glme_encode_uint64
 */
extern int glme_encode_int(glme_buf_t *gbuf, const int *v);
extern int glme_encode_value_int(glme_buf_t *gbuf, const int *v);

/**
 * Encode single precision float into the specified buffer.
 *
 * @see glme_encode_uint64
 */
extern int glme_encode_float(glme_buf_t *gbuf, const float *v);
extern int glme_encode_value_float(glme_buf_t *gbuf, const float *v);

/**
 * Encode single precision complex into the specified buffer.
 *
 * @see glme_encode_uint64
 */
extern int glme_encode_complex64(glme_buf_t *gbuf, const float complex *v);
extern int glme_encode_value_complex64(glme_buf_t *gbuf, const float complex *v);

/**
 * Encode uninterpreted byte stream value into the specified buffer.
 *
 * @see glme_encode_uint64
 */
extern int glme_encode_bytes(glme_buf_t *gbuf, const void *s, size_t len);

/**
 * Encode vector or string type into the specified buffer.
 *
 * @see glme_encode_uint64
 */
extern int glme_encode_string(glme_buf_t *gbuf, const char *s);
extern int glme_encode_vector(glme_buf_t *gbuf, const char *s, size_t len);


/**
 * Encode array start into the specified buffer. Array header consists of element
 * typeid and unsigned number of elements.
 *
 * @see glme_encode_uint64
 */
extern int glme_encode_array_header(glme_buf_t *gbuf, int typeid, size_t sz);

/**
 * Encode array data elements into the specified buffer.
 *
 * @param  enc    Encode buffer
 * @param  vptr   Array elements
 * @param  len    Number of elements
 * @param  esize  Element size in bytes
 * @param  efunc  Element value encoding function
 */
extern int glme_encode_array_data(glme_buf_t *enc, const void *vptr,
                                  size_t len, size_t esize, glme_encoder_f efunc);

/**
 * Encode array with given element type into the specified buffer.
 *
 * @param   gbuf    Encode buffer
 * @param   typeid  Array element typeid
 * @param   vptr    Array pointer
 * @param   len     Number of elements in the array
 * @param   esize   Array element size
 * @param   efunc   Element value encoding function
 *
 * @return
 *    Negative error code or number of bytes writen to encode buffer.
 */
extern int glme_encode_array(glme_buf_t *gbuf, int typeid, const void *vptr,
                             size_t len, size_t esize, glme_encoder_f efunc);
extern int glme_encode_value_array(glme_buf_t *enc, int typeid, const void *vptr,
                                   size_t len,  size_t esize, glme_encoder_f efunc);

/**
 * Encode struct start into the specified buffer.
 *
 * @see glme_encode_uint64
 */
extern int glme_encode_start_struct(glme_buf_t *gbuf, int *delta);

/**
 * Encode structure field into the specified buffer
 *
 * @param   gbuf    Encode buffer
 * @param   delta   Pointer to field counter delta
 * @param   typeid  Element type, if array then array element typeid
 * @param   flags   Flag bits 
 * @param   vptr    Pointer to field value
 * @param   nlen    Number of elements in array
 * @param   esize   Element size, used also not-empty value indicator for simple types.
 * @param   efunc   Element value encoder function
 *
 */
extern int glme_encode_field(glme_buf_t *gbuf, int *delta, int typeid, int flags,
                             const void *vptr, size_t nlen, size_t esize, glme_encoder_f efunc);

/**
 * Encode structure end mark into the specified buffer.
 */
extern int glme_encode_end_struct(glme_buf_t *gbuf);

/**
 * Encode structure into the specified buffer.
 */
extern int glme_encode_struct(glme_buf_t *enc, int typeid, const void *ptr, glme_encoder_f efunc);

/**
 * Encode type id into the specified buffer.
 */
extern int glme_encode_type(glme_buf_t *gbuf, int typeid);

/**
 * Decode unsigned 64 bit integer type or value from the specified decoder.
 *
 * @param dec
 *   Decoder
 * @param v
 *   Pointer to value store location.
 * @return
 *   Number of bytes read from decoder. Negative value indicates 
 *   underflow and number of bytes needed to decode value.
 */
extern int glme_decode_uint64(glme_buf_t *dec, uint64_t *v);
extern int glme_decode_value_uint64(glme_buf_t *dec, uint64_t *v);


/**
 * Decode unsigned 64 bit integer from the specified decoder without
 * moving internal read pointers.
 *
 * @see glme_decode_uint64
 */
extern int glme_decode_peek_uint64(glme_buf_t *dec, uint64_t *v);

/**
 * Decode signed 64 bit integer type or value from the specified decoder.
 *
 * @param dec
 *   Decoder
 * @param v
 *   Pointer to value store location.
 */
extern int glme_decode_int64(glme_buf_t *dec, int64_t *v);
extern int glme_decode_value_int64(glme_buf_t *dec, int64_t *v);

/**
 * Read signed 64 bit integer from the specified decoder without
 * moving internal read pointers.
 *
 */
extern int glme_decode_peek_int64(glme_buf_t *dec, int64_t *v);


/**
 * Decode double precision IEEE floating point type or valuer from the
 * specified decoder.
 *
 * @param dec
 *   Decoder
 * @param v
 *   Pointer to value store location.
 */
extern int glme_decode_double(glme_buf_t *dec, double *v);
extern int glme_decode_value_double(glme_buf_t *dec, double *v);

extern int glme_decode_float(glme_buf_t *dec, float *v);
extern int glme_decode_value_float(glme_buf_t *dec, float *v);

/**
 * Decode double precision IEEE complex type or valuer from the
 * specified decoder.
 *
 * @param dec
 *   Decoder
 * @param v
 *   Pointer to value store location.
 */
extern int glme_decode_complex128(glme_buf_t *dec, double complex *v);
extern int glme_decode_value_complex128(glme_buf_t *dec, double complex *v);

extern int glme_decode_complex64(glme_buf_t *dec, float complex *v);
extern int glme_decode_value_complex64(glme_buf_t *dec, float complex *v);


/**
 * Decode unsigned long from the specified decoder.
 *
 * @see glme_decode_uint64
 */
extern int glme_decode_ulong(glme_buf_t *dec, unsigned long *u);
extern int glme_decode_value_ulong(glme_buf_t *dec, unsigned long *u);

/**
 * Decode signed long type or value from the specified decoder.
 *
 * @see glme_decode_int64
 */
extern int glme_decode_long(glme_buf_t *dec, long *d);
extern int glme_decode_value_long(glme_buf_t *dec, long *d);


/**
 * Decode unsigned int type or value from the specified decoder.
 *
 * @see glme_decode_uint64
 */
extern int glme_decode_uint(glme_buf_t *dec, unsigned int *u);
extern int glme_decode_value_uint(glme_buf_t *dec, unsigned int *u);

/**
 * Decode signed int type or value from the specified decoder.
 *
 * @see glme_decode_int64
 */
extern int glme_decode_int(glme_buf_t *dec, int *d);
extern int glme_decode_value_int(glme_buf_t *dec, int *d);

/**
 * Decode byte array or string into a fixed length bytes vector
 * from the specified decoder.
 *
 * If destination buffer length less than encoded buffer length then
 * only len bytes are copied from the buffer. If encoded length less
 * buffer length then extra space in buffer is zeroed.
 *
 * Stream: [<GLME_VECTOR|GLME_STRING>, <length> <data>]
 */
extern int glme_decode_vector(glme_buf_t *dec, void *s, size_t len);

/**
 * Decode data of variable length byte array from the specified decoder.
 * Allocates space for decoded data if len is zero. Otherwise 's' is assumed
 * contain pointer to allocated buffer of size 'len'.
 *
 * stream: [<length> <data>]
 */
extern int glme_decode_bytes(glme_buf_t *dec, void **s, size_t len);

/**
 * Decode variable length string from the specified decoder.
 *
 * stream: [<GLME_STRING> <length> <data>]
 */
extern int glme_decode_string(glme_buf_t *dec, char **s);

/**
 * Read (peek) start of struct marker from the specified decoder.
 *
 * @return
 *   Length of the start struct header.
 *
 * stream: [<GLME_STRUCT> <typeid> ....]
 */
extern int glme_decode_start_struct(glme_buf_t *dec, int *typeid);
extern int glme_decode_peek_struct(glme_buf_t *dec, int *typeid);

/**
 * Read next field number from the decoder.
 *
 * @param dec   Decoder
 * @param next  Next field number
 * @param cur   Current field number
 *
 * Field offset is read from the buffer and value of next is set to cur + offset - 1.
 * If offset is zero then next is set to -1.
 *
 * @return
 *    Number of bytes consumed or negative error number.
 */
extern int glme_decode_field_id(glme_buf_t *dec, int *next, int cur);

/**
 * Read next field number from the decoder.
 *
 * @param dec    Decoder
 * @param delta  Current delta value
 * @param typeid Expected type id
 * @param flags  Flag bits
 * @param vptr   Element pointer
 * @param nlen   Number of elements in array
 * @param esize  Element size
 * @param dfunc  Element decoder function
 *
 * @return
 *    Number of bytes consumed or negative error number.
 */
extern int glme_decode_field(glme_buf_t *dec, unsigned int *delta, int typeid, int flags,
                             void *vptr, size_t *nlen, size_t esize, glme_decoder_f dfunc);

/**
 * Initialize structure decoder
 */
extern int glme_decode_start_struct(glme_buf_t *dec, int *delta);
/**
 * Read end of struct marker from the specified decoder.
 */
extern int glme_decode_end_struct(glme_buf_t *dec);

/**
 * Read structure type from the specified buffer.
 *
 * @param dec
 *   Decoder
 * @param typeid
 *   Type id of expected structure. 
 * @param dptr
 *   Pointer to structure pointer. If structure pointer is null space is allocated for the structure.
 * @param esize
 *   Structure size in bytes
 * @param dfunc
 *   Decoder function
 *
 * If expected type id is non-zero then decoded type id is checked and error reported if they do not
 * match. If structure pointer is null then esize bytes of memory is allocated to hold the
 * decoded structure.  If esize is zero then type size for the decoded type id is looked up from
 * internal type registery. If lookup fails then error is returned. Likewise if decoder function
 * is null it is looked up from the registery and error is returned if decoder function not found.
 *
 * @return
 *   Number of bytes consumed or negative error code.
 */
extern int glme_decode_struct(glme_buf_t *dec, int typeid, void **dptr, size_t esize, glme_decoder_f dfunc);

/**
 * Read structure value from the specified buffer.
 *
 * @param dec
 *   Decoder
 * @param dptr
 *   Pointer to structure pointer. If structure pointer is null space is allocated and esize
 *   parameter must be non-zero. 
 * @param esize
 *   Structure size in bytes
 * @param dfunc
 *   Decoder function, must not be null.
 *
 * @return
 *   Number of bytes consumed or negative error code.
 */
extern int glme_decode_value_struct(glme_buf_t *dec, void **dptr, size_t esize, glme_decoder_f dfunc);

/**
 * Read start of array from the specified decoder.
 *
 * Array start is ARRAY typeid, element typeid and unsigned count of elements.
 */
extern int glme_decode_array_start(glme_buf_t *dec, int *typeid, size_t *len);

/**
 * Read array header from the specified decoder.
 *
 * Array header is element typeid and unsigned count of elements.
 */
extern int glme_decode_array_header(glme_buf_t *dec, int *typeid, size_t *len);

/**
 * Read array data from the specified decoder.
 *
 * @param dec    Decoder
 * @param dst    Target array. If null then space is allocated.
 * @param nlen   Number of elements in array
 * @param esize  Element size
 * @param func   Element decoder function

 * @return
 *   Number of bytes read or negative error code.
 */
extern int glme_decode_array_data(glme_buf_t *dec, void **dst,
                                  size_t len, size_t esize, glme_decoder_f func);

/**
 * Read array from the specified buffer.
 */
extern int glme_decode_array(glme_buf_t *dec, int *typeid, void **dst,
                             size_t *len, size_t esize, glme_decoder_f func);

/**
 * Read array value from the specified buffer. (Array sans ARRAY typeid)
 */
extern int glme_decode_value_array(glme_buf_t *dec, int *typeid, void **dst,
                                   size_t *len, size_t esize, glme_decoder_f func);

/**
 * Read type id from the specified buffer.
 */
extern int glme_decode_type(glme_buf_t *dec, int *t);

/**
 * Read type id from the specified buffer.
 *
 * @return
 *   Positive non-zero number bytes read or negative value if
 *   type id does not match parameter typeid.
 */
extern int glme_decode_peek_type(glme_buf_t *dec, int *typeid);

// ----------------------------------------------------------------------------
// encode helper macros

/**
 * Encoder standard definitions.
 */
#define GLME_ENCODE_STDDEF(enc)                 \
  int __e, __ne, __delta;                       \
  size_t __nl, __start_at = (enc)->count


/**
 * Encode structure header
 *
 * @param enc    Encode buffer
 * @param typeid Type id for structure type.
 */
#define GLME_ENCODE_STRUCT_START(enc) \
  do { \
    if (glme_encode_start_struct(enc, &__delta) < 0)	\
      return -1;					\
  } while (0)

/**
 * Encode structure end marker. Make return with number of bytes encoded.
 *
 * @param enc    Encode buffer
 */
#define GLME_ENCODE_STRUCT_END(enc)              \
  do {                                           \
    if (glme_encode_end_struct(enc) < 0)         \
      return -4;                                 \
  } while (0)

#define GLME_ENCODE_RETURN(enc) \
  return (enc)->count - __start_at

/**
 * Encode signed integer
 *
 * @param enc    Encode buffer
 * @param elem   Signed integer element
 * @param defval Default value, field omitted if it's value is equal to defval
 */
#define GLME_ENCODE_FLD_INT(enc, elem, defval)                        \
  do {                                                                \
    int64_t __i64 = (int64_t)(elem);                                  \
    __ne = (elem) != defval;                                          \
    __e = glme_encode_field(enc, &__delta, GLME_INT, 0, &__i64, 0,    \
                            __ne, (glme_encoder_f)glme_encode_int64); \
    if (__e < 0) return __e;                                          \
  } while (0)

/**
 * Encode unsigned integer
 *
 * @param enc    Encode buffer
 * @param elem   Unsigned integer element
 * @param defval Default value, field omitted if it's value is equal to defval
 */
#define GLME_ENCODE_FLD_UINT(enc, elem, defval)                     \
  do {                                                              \
    uint64_t __u64 = (uint64_t)(elem);                              \
    __ne = (elem) != defval;                                        \
    __e = glme_encode_field(enc, &__delta, GLME_UINT, 0, &__u64, 0, \
                            __ne, (glme_encoder_f)glme_encode_uint64);     \
    if (__e < 0) return __e;                                        \
  } while (0)

/**
 * Encode floating point number
 *
 * @param enc    Encode buffer
 * @param fno    Field number (for error codes)
 * @param elem   Floating point element
 * @param defval Default value, field omitted if it's value is equal to defval
 */
#define GLME_ENCODE_FLD_DOUBLE(enc, elem, defval)                    \
  do {                                                               \
    double __f64 = (double)(elem);                                   \
    __ne = (elem) != defval;                                         \
    __e = glme_encode_field(enc, &__delta, GLME_FLOAT, 0, &__f64, 0, \
                            __ne, (glme_encoder_f)glme_encode_double);      \
    if (__e < 0) return __e;                                         \
  } while (0)


/**
 * Encode null terminated string.
 *
 * @param enc   Encode buffer
 * @param elem  Pointer to string element
 */
#define GLME_ENCODE_FLD_STRING(enc, elem)                  \
  do {                                                     \
    __e = glme_encode_field(enc, &__delta, GLME_STRING, 0, \
                            (elem), 0, 1, (glme_encoder_f)0);     \
    if (__e < 0) return __e;                               \
  } while (0)

/**
 * Encode byte vector of specified length.
 *
 * @param enc   Encode buffer
 * @param elem  Vector or pointer to vector
 * @param len   Number of bytes in vector
 */
#define GLME_ENCODE_FLD_VECTOR(enc, elem, len)                \
  do {                                                        \
    __ne = (len) > 0 ? 1 : 0;                                 \
    __e = glme_encode_field(enc, &__delta, GLME_VECTOR, 0,    \
                            (elem), (len), __ne, (glme_encoder_f)0); \
    if (__e < 0) return __e;                                  \
  } while (0)

/**
 * Encode structure that element points to.
 *
 * @param enc    Encode buffer
 * @param typeid Structure type id
 * @param elem   Structure pointer
 * @param func   Encoding function for structure
 */
#define GLME_ENCODE_FLD_STRUCT(enc, typeid, elem, func)           \
  do {                                                            \
    __e = glme_encode_field(enc, &__delta, typeid, 0, (elem),     \
                            0, sizeof((elem)[0]), func);          \
    if (__e < 0) return __e;                                      \
  } while (0)


#if 0
/**
 * Encode embedded structure.
 *
 * @param enc   Encode buffer
 * @param fno   Field number (for error codes)
 * @param elem  Embedded structure element
 * @param func  Encoding function for structure
 */
#define GLME_ENCODE_FLD_STRUCT(enc, fno, elem, func)	 \
  do {							 \
    if (glme_encode_value_uint64(enc, &__delta) < 0)	 \
      return -(10+fno);					 \
    if ((*func)(enc, &(elem)) < 0)			 \
      return -(10+fno);					 \
    __delta = 1;					 \
  } while (0)
#endif

/**
 * Encode array of signed integers.
 *
 * @param enc    Encode buffer
 * @param fno    Field number
 * @param elem   Source array
 * @param len    Number of element in source
 */
#define GLME_ENCODE_FLD_INT_ARRAY(enc, elem, len, vfunc)        \
  do {								\
      __e = glme_encode_field(enc, &__delta,                    \
                              GLME_INT, GLME_F_ARRAY,           \
                              (elem), (len), sizeof((elem)[0]), \
                              (glme_encoder_f)vfunc);                  \
      if (__e < 0) return __e;                                  \
  } while (0)

#define GLME_ENCODE_FLD_INT_VECTOR(enc, elem, vfunc)            \
  do {								\
    __nl = sizeof(elem)/sizeof((elem)[0]);                      \
    __e = glme_encode_field(enc, &__delta,                      \
                            GLME_INT, GLME_F_ARRAY,             \
                            (elem), __nl, sizeof((elem)[0]),    \
                            (glme_encoder_f)vfunc);                    \
    if (__e < 0) return __e;                                    \
  } while (0)


/**
 * Encode array of unsigned integers.
 *
 * @param enc    Encode buffer
 * @param elem   Source array
 * @param len    Number of element in source
 */
#define GLME_ENCODE_FLD_UINT_ARRAY(enc, elem, len, vfunc)	\
  do {								\
      __e = glme_encode_field(enc, &__delta,                    \
                              GLME_UINT, GLME_F_ARRAY,          \
                              (elem), (len), sizeof((elem)[0]), \
                              (glme_encoder_f)vfunc);                  \
      if (__e < 0) return __e;                                  \
  } while (0)

#define GLME_ENCODE_FLD_UINT_VECTOR(enc, elem, vfunc)           \
  do {								\
    __nl = sizeof(elem)/sizeof((elem)[0]);                      \
    __e = glme_encode_field(enc, &__delta,                      \
                            GLME_UINT, GLME_F_ARRAY,           \
                            (elem), __nl, sizeof((elem)[0]),    \
                            (glme_encoder_f)vfunc);                    \
    if (__e < 0) return __e;                                    \
  } while (0)

/**
 * Encode array of unsigned integers.
 *
 * @param enc    Encode buffer
 * @param elem   Source array
 * @param len    Number of element in source
 */
#define GLME_ENCODE_FLD_FLOAT_ARRAY(enc, elem, len, vfunc)	\
  do {								\
      __e = glme_encode_field(enc, &__delta,                    \
                              GLME_FLOAT, GLME_F_ARRAY,        \
                              (elem), (len), sizeof((elem)[0]), \
                              (glme_encoder_f)vfunc);                  \
      if (__e < 0) return __e;                                  \
  } while (0)

#define GLME_ENCODE_FLD_FLOAT_VECTOR(enc, elem, vfunc)          \
  do {								\
    __nl = sizeof(elem)/sizeof((elem)[0]);                      \
    __e = glme_encode_field(enc, &__delta,                      \
                            GLME_FLOAT, GLME_F_ARRAY,           \
                            (elem), __nl, sizeof((elem)[0]),    \
                            (glme_encoder_f)vfunc);                    \
    if (__e < 0) return __e;                                    \
  } while (0)


/**
 * Insert code for starting encoding header of an array field.
 * If len is zero field is omitted and code between START_ARRAY and
 * corresponding END_ARRAY is not executed.
 */
#define GLME_ENCODE_FLD_START_ARRAY(enc, tag, typeid, len)    \
  do {                                                       \
    if (len == 0) {                                          \
      __delta++;                                             \
      goto __empty_array_ ## tag;                            \
    }                                                        \
    if ((__e=glme_encode_value_uint(enc, &__delta)) < 0)     \
      return __e;                                            \
    if ((__e=glme_encode_array_start(enc, typeid, len)) < 0) \
      return __e;                                            \
  } while (0)

/**
 * Insert code to end array field encoding block. Matching START_ARRAY block
 * is identified with tag value.
 */
#define GLME_ENCODE_FLD_END_ARRAY(enc, tag)   \
  __delta = 1;                                \
  __empty_array_ ## tag:                      \
  do {} while (0)

// ----------------------------------------------------------------------------
// decode macros

/**
 * Decoder standard definitions.
 */
#define GLME_DECODE_STDDEF(dec)            \
  int __e, __delta, __flg;                 \
  size_t __len, __nl;                      \
  uint64_t __at_start = (dec)->current

/**
 * Decode structure header.
 *
 * @param dec    Decode buffer
 */
#define GLME_DECODE_STRUCT_START(dec)                   \
  do {							\
    if (glme_decode_start_struct(dec, &__delta) < 0)    \
      return -1;					\
  } while (0)
  
/**
 * Decode end of structure marker.
 */
#define GLME_DECODE_STRUCT_END(dec)       \
  do {                                    \
    if (glme_decode_end_struct(dec) < 0)  \
      return -4;			  \
  } while (0)

#define GLME_DECODE_RETURN(dec) \
    return (dec)->current - __at_start

/**
 * Decode signed integer value.
 *
 * @param dec     Decode buffer
 * @param elem    Element, must be proper LHS.
 * @param defval  Default value. If field omitted from stream, element set to this value.
 */
#define GLME_DECODE_FLD_INT(dec, elem, defval)                     \
  do {                                                             \
    int64_t __t = (int64_t)(defval);                               \
    __e = glme_decode_field(dec, &__delta, GLME_INT, 0, &__t,      \
                            &__nl, 1, (glme_decoder_f)glme_decode_int64); \
    if (__e < 0) return __e;                                       \
    (elem) = __t;                                                  \
  } while(0)

/**
 * Decode unsigned integer value.
 *
 * @param dec     Decode buffer
 * @param elem    Element, must be proper LHS.
 * @param defval  Default value. If field omitted from stream, element set to this value.
 */
#define GLME_DECODE_FLD_UINT(dec, elem, defval)                     \
  do {                                                              \
    uint64_t __t = (uint64_t)(defval);                              \
    __e = glme_decode_field(dec, &__delta, GLME_UINT, 0, &__t,      \
                            &__nl, 1, (glme_decoder_f)glme_decode_uint64); \
    if (__e < 0) return __e;                                        \
    (elem) = __t;                                                   \
  } while(0)

/**
 * Decode floating point number value.
 *
 * @param dec     Decode buffer
 * @param elem    Element, must be proper LHS.
 * @param defval  Default value. If field omitted from stream, element set to this value.
 */
#define GLME_DECODE_FLD_DOUBLE(dec, elem, defval)                   \
  do {                                                              \
    double __f = (double)(defval);                                  \
    __e = glme_decode_field(dec, &__delta, GLME_FLOAT, 0, &__f,     \
                            &__nl, 1, (glme_decoder_f)glme_decode_double); \
    if (__e < 0) return __e;                                          \
    (elem) = __f;                                                     \
  } while(0)

/**
 * Decode fixed size byte array
 *
 * @param dec     Decode buffer
 * @param elem    Element array of type char[]
 */
#define GLME_DECODE_FLD_VECTOR(dec, elem)                           \
  do {                                                              \
    void *__ptr = &(elem); __nl = sizeof(elem)/sizeof((elem)[0]);   \
    memset((elem), 0, sizeof(elem));                                \
    __e = glme_decode_field(dec, &__delta, GLME_VECTOR, 0,          \
                          __ptr, &__nl, 1, (glme_decoder_f)0);             \
    if (__e < 0) return __e;                                        \
  } while(0)

/**
 * Decode variable string
 *
 * @param dec     Decode buffer
 * @param elem    Element, string pointer
 */
#define GLME_DECODE_FLD_STRING(dec, elem)                           \
  do {                                                              \
    void *__ptr = &(elem); __nl = 0;                                \
    (elem) = (char *)0;                                             \
    __e = glme_decode_field(dec, &__delta, GLME_STRING, 0,          \
                            &(elem), &__nl, 1, (glme_decoder_f)0);         \
    if (__e < 0) return __e;                                        \
  } while(0)


/**
 * Decode structure to a pointer field.
 *
 * @param dec     Decode buffer
 * @param typeid  Structure type id
 * @param elem    Element, structure pointer
 * @param func    Decode function
 */
#define GLME_DECODE_FLD_STRUCT_PTR(dec, typeid, elem, func)             \
  do {                                                                  \
    (elem) = (void *)0;                                                 \
    __e = glme_decode_field(dec, &__delta, typeid, GLME_F_PTR, &(elem), \
                            0, sizeof((elem)[0]), (glme_decoder_f)func);       \
    if (__e < 0) return __e;                                            \
  } while (0)

/**
 * Decode structure to an embedded structure field.
 *
 * @param dec     Decode buffer
 * @param typeid  Structure type id
 * @param elem    Element
 * @param func    Decode function
 */
#define GLME_DECODE_FLD_STRUCT(dec, typeid, elem, func)         \
  do {                                                          \
    memset(&(elem), 0, sizeof(elem));                           \
    __e = glme_decode_field(dec, &__delta, typeid, 0, &(elem),  \
                            0, sizeof(elem), (glme_decoder_f)func);    \
    if (__e < 0) return __e;                                    \
  } while (0)


/**
 * Decode array of signed integers.
 *
 * @param dec    Decode buffer
 * @param elem   Target field, must be proper lvalue
 * @param len    Decoded array length (must be lvalue)
 * @param func   Array element value decoder
 */
#define GLME_DECODE_FLD_INT_ARRAY(dec, elem, len, func)                 \
  do {                                                                  \
    __flg = GLME_F_ARRAY|GLME_F_PTR; (len) =  0;                        \
    __e = glme_decode_field(dec, &__delta, GLME_INT, __flg, &(elem),    \
                            &(len), sizeof((elem)[0]), (glme_decoder_f)func);  \
    if (__e < 0) return __e;                                            \
  } while (0)


/**
 * Decode fixed size array of signed integers
 *
 * @param dec    Decode buffer
 * @param elem   Fixed size target array
 * @param len    Target array length
 * @param func   Array element value decoder
 */
#define GLME_DECODE_FLD_INT_VECTOR(dec, elem, func)                 \
  do {                                                              \
    void *__ptr = &(elem); __nl = sizeof(elem)/sizeof((elem)[0]);   \
    memset((elem), 0, sizeof(elem));                                \
    __e = glme_decode_field(dec, &__delta, GLME_INT, GLME_F_ARRAY,  \
                            &(elem), &__nl, sizeof((elem)[0]),      \
                            (glme_decoder_f)func);                         \
    if (__e < 0) return __e;                                        \
  } while (0)


/**
 * Decode array of unsigned integers.
 *
 * @param dec    Decode buffer
 * @param elem   Target field, must be proper lvalue
 * @param len    Decoded array length (must be lvalue)
 * @param func   Array element value decoder
 */
#define GLME_DECODE_FLD_UINT_ARRAY(dec, elem, len, func)                \
  do {                                                                  \
    __flg = GLME_F_ARRAY|GLME_F_PTR; (len) =  0;                        \
    __e = glme_decode_field(dec, &__delta, GLME_UINT, __flg, &(elem),   \
                            &(len), sizeof((elem)[0]), (glme_decoder_f)func);  \
    if (__e < 0) return __e;                                            \
  } while (0)

/**
 * Decode fixed size array of unsigned integers
 *
 * @param dec    Decode buffer
 * @param elem   Fixed size target array
 * @param len    Target array length
 * @param func   Array element value decoder
 */
#define GLME_DECODE_FLD_UINT_VECTOR(dec, elem, func)                \
  do {                                                              \
    void *__ptr = &(elem); __nl = sizeof(elem)/sizeof((elem)[0]);   \
    memset((elem), 0, sizeof(elem));                                \
    __e = glme_decode_field(dec, &__delta, GLME_UINT, GLME_F_ARRAY, \
                            &(elem), &__nl, sizeof((elem)[0]),      \
                            (glme_decoder_f)func);                         \
    if (__e < 0) return __e;                                        \
  } while (0)

/**
 * Decode array of floating point numbers.
 *
 * @param dec    Decode buffer
 * @param elem   Target field, must be proper lvalue (pointer)
 * @param len    Decoded array length (must be lvalue)
 * @param func   Array element value decoder
 */
#define GLME_DECODE_FLD_FLOAT_ARRAY(dec, elem, len, func)              \
  do {                                                                 \
    __flg = GLME_F_ARRAY|GLME_F_PTR; (len) =  0;                       \
    __e = glme_decode_field(dec, &__delta, GLME_FLOAT, __flg, &(elem), \
                            &(len), sizeof((elem)[0]), (glme_decoder_f)func); \
    if (__e < 0) return __e;                                           \
  } while (0)

/**
 * Decode fixed size array of floating point numbers.
 *
 * @param dec    Decode buffer
 * @param elem   Fixed size target array
 * @param func   Array element value decoder
 */
#define GLME_DECODE_FLD_FLOAT_VECTOR(dec, elem, func)                  \
  do {                                                                 \
    void *__ptr = &(elem)[0]; __nl = sizeof(elem)/sizeof((elem)[0]);   \
    memset((elem), 0, sizeof(elem));                                   \
    __e = glme_decode_field(dec, &__delta, GLME_FLOAT, GLME_F_ARRAY,   \
                            &__ptr, &__nl, sizeof((elem)[0]),          \
                            (glme_decoder_f)func);                     \
    if (__e < 0) return __e;                                           \
  } while (0)

/**
 * Insert start of array field code. Code between this and matching
 * END_ARRAY macros is not executed if actual array length is zero
 * and array is omitted from encoded stream.
 */
#define GLME_DECODE_FLD_START_ARRAY(dec, id, typeid, len)	\
    do {							\
      int __typ; size_t __alen;					\
      if (__delta != 1) {					\
        if (__delta > 0) __delta--;				\
	(len) = 0;						\
	goto __skip_array_ ## fno;				\
      }								\
      __e = glme_decode_value_uint(dec, &__delta);              \
      if (__e < 0) return __e;                                  \
      __e = glme_decode_array_start(dec, &__typ, &__alen);	\
      if (__e < 0) return __e;                                  \
      if (__typ != typeid)					\
	return -4;                                              \
      (len) = __alen;						\
    } while (0)

/**
 * Insert end of array field code.
 */
#define GLME_DECODE_FLD_END_ARRAY(dec, id)          \
    __skip_array_ ## fno:			     \
    do {} while (0)
    


#ifdef __cplusplus
}
#endif

#endif  // __GLME_H


// Local Variables:
// indent-tabs-mode: nil
// End:
