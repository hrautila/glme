
// Copyright (c) Harri Rautila, 2015

// GLME is Gob Like Message Encoding

#ifndef __GLME_DECODER_H
#define __GLME_DECODER_H

#include <stdint.h>

#ifndef __INLINE__
#define __INLINE__ extern inline
#endif


/**
 * Gob Like Message decoder structure
 */
typedef struct glme_decoder {
  char *buf;
  size_t buflen;	// size of buffer in bytes
  size_t count;		// number of data bytes in buffer
  size_t current;	// current read pointer (current <= count <= buflen)
  int owner;
} glme_decoder_t;

/**
 * Initialize the specified decoder with buffer space of len bytes.
 *
 * @param dec
 *   The decoder.
 * @param len
 *   Requested initial buffer space in bytes.
 *
 * @return
 *   Initialized decoder.
 */
__INLINE__
glme_decoder_t *glme_decoder_init(glme_decoder_t *dec, size_t len)
{
  if (dec) {
    dec->count = dec->current = 0;
    // allow initialization to zero size
    if (len > 0) {
      dec->buf = malloc(len);
      if (! dec->buf)
	return (glme_decoder_t *)0;
      dec->owner = 1;
    } else {
      dec->buf = (char *)0;
    }
    dec->buflen = dec->buf ? len : 0;
  }
  return dec;
}

/**
 * Make decoder from spesified data buffer.
 *
 * @param dec
 *   Decoder
 * @param data
 *   Data buffer
 * @param len
 *   Length of the buffer
 * @param count
 *   Length of the encoded content in the data buffer
 * @return
 *   Initialized decoder
 */
__INLINE__
glme_decoder_t *glme_decoder_make(glme_decoder_t *dec, char *data, size_t len, size_t count)
{
  dec->buf = data;
  dec->buflen = len;
  dec->count = count;
  dec->current = 0;
  dec->owner = 0;
  return dec;
}

/**
 * Create new decoder with buffer space of len bytes.
 *
 * @param len
 *   Requested initial buffer space in bytes.
 *
 * @return
 *   New initialized decoder.
 */
__INLINE__
glme_decoder_t *glme_decoder_new(size_t len)
{
  glme_decoder_t *dec = malloc(sizeof(glme_decoder_t));
  if (dec)
    return glme_decoder_init(dec, len);
  return (glme_decoder_t *)0;
}

/**
 * Close the decoder. Releases allocated buffer and reset read pointers.
 */
__INLINE__
void glme_decoder_close(glme_decoder_t *dec)
{
  if (dec) {
    if (dec->buf && dec->owner)
      free(dec->buf);
    dec->buf = (char *)0;
    dec->buflen = 0;
    dec->count = 0;
    dec->current = 0;
  }
}

/**
 * Release the decoder.
 */
__INLINE__
void glme_decoder_free(glme_decoder_t *dec)
{
  if (dec) {
    if (dec->buf && dec->owner)
      free(dec->buf);
    dec->buflen = 0;
    dec->count = 0;
    dec->current = 0;
    free(dec);
  }
}

/**
 * Reset decoders read pointer.
 */
__INLINE__
void glme_decoder_reset(glme_decoder_t *dec)
{
  if (dec)
    dec->current = 0;
}


/**
 * Increase size of the decoder buffer.
 *
 * @param dec
 *   The decoder
 * @parm increase
 *   Number of bytes to increase the buffer
 */
__INLINE__
void glme_decoder_resize(glme_decoder_t *dec, size_t increase)
{
  char *b = malloc(dec->buflen + increase);
  if (b) {
    if (dec->buf) {
      // copy if we have buffer and content in it.
      if (dec->count > 0)
	memcpy(b, dec->buf, dec->count);
      free(dec->buf);
    }
    dec->buf = b;
    dec->buflen += increase;
  }
}

/**
 * Construct decoder from the speficied buffer. 
 */
__INLINE__
void glme_decoder_setbuf(glme_decoder_t *dec, char *buf, size_t buflen)
{
  if (dec) {
    dec->buf = buf;
    dec->buflen = buflen;
    dec->count = dec->current = 0;
  }
}

/**
 * Read message from the specified file descriptor into the decoder.
 *
 * @return
 *   Number of bytes read or -1 for error.
 */
int glme_decoder_readm(glme_decoder_t *dec, int fd);

/**
 * Decode unsigned 64 bit integer from the specified decoder.
 *
 * @param dec
 *   Decoder
 * @param v
 *   Pointer to value store location.
 * @return
 *   Number of bytes read from decoder. Negative value indicates 
 *   underflow and number of bytes needed to decode value.
 */
int glme_decode_uint64(glme_decoder_t *dec, uint64_t *v);


/**
 * Decode unsigned 64 bit integer from the specified decoder without
 * moving internal read pointers.
 *
 * @see glme_decode_uint64
 */
int glme_decode_uint64_peek(glme_decoder_t *dec, uint64_t *v);

/**
 * Decode signed 64 bit integer from the specified decoder.
 *
 * @param dec
 *   Decoder
 * @param v
 *   Pointer to value store location.
 */
int glme_decode_int64(glme_decoder_t *dec, int64_t *v);

/**
 * Read signed 64 bit integer from the specified decoder without
 * moving internal read pointers.
 *
 */
int glme_decode_int64_peek(glme_decoder_t *dec, int64_t *v);


/**
 * Decode double precision IEEE floating point number from the specified decoder.
 *
 * @param dec
 *   Decoder
 * @param v
 *   Pointer to value store location.
 */
int glme_decode_double(glme_decoder_t *dec, double *v);

__INLINE__
int glme_decode_float(glme_decoder_t *dec, float *v)
{
  int n;
  double dv;
  n = glme_decode_double(dec, &dv);
  *v = n < 0 ? 0.0 : (float)dv;
  return n;
}

/**
 * Decode fixed length bytes stream from the specified decoder.
 */
int glme_decode_vec(glme_decoder_t *dec, void *s, size_t len);

/**
 * Decode variable length byte array from the specified decoder.
 * Allocates space for decoded data.
 */
int glme_decode_bytes(glme_decoder_t *dec, void **s);

/**
 * Decode variable length string from the specified decoder.
 */
int glme_decode_string(glme_decoder_t *dec, char **s);

/**
 * Decode unsigned long from the specified decoder.
 *
 * @see glme_decode_uint64
 */
__INLINE__
int glme_decode_ulong(glme_decoder_t *dec, unsigned long *u)
{
  int n;
  uint64_t u64;
  n = glme_decode_uint64(dec, &u64);
  *u = n < 0 ? 0 : (unsigned long)u64;
  return n;
}

/**
 * Decode signed long from the specified decoder.
 *
 * @see glme_decode_int64
 */
__INLINE__
int glme_decode_long(glme_decoder_t *dec, long *d)
{
  int n;
  int64_t i64;
  n = glme_decode_int64(dec, &i64);
  *d = n < 0 ? 0 : (long)i64;
  return n;
}


/**
 * Decode unsigned int from the specified decoder.
 *
 * @see glme_decode_uint64
 */
__INLINE__
int glme_decode_uint(glme_decoder_t *dec, unsigned int *u)
{
  int n;
  uint64_t u64;
  n = glme_decode_uint64(dec, &u64);
  *u = n < 0 ? 0 : (unsigned int)u64;
  return n;
}

/**
 * Decode signed int from the specified decoder.
 *
 * @see glme_decode_int64
 */
__INLINE__
int glme_decode_int(glme_decoder_t *dec, int *d)
{
  int n;
  int64_t i64;
  n = glme_decode_int64(dec, &i64);
  *d = n < 0 ? 0 : (int)i64;
  return n;
}

/**
 * Read end of struct marker from the specified decoder.
 *
 * @return
 *   Length of the marker (1 bytes) or negative error code if next
 *   is not end-of-struct marker.
 */
int glme_decode_end_struct(glme_decoder_t *dec);

/**
 * Push read pointer backwards the specified number of bytes.
 */
__INLINE__
void glme_decode_pushback(glme_decoder_t *dec, size_t count)
{
  if (count > dec->current) {
    dec->current = 0;
  } else {
    dec->current -= count;
  }
}
#endif

// Local Variables:
// indent-tabs-mode: nil
// End:
