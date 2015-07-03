
// Copyright (c) Harri Rautila, 2015

// GLME is Gob Like Message

#ifndef __GLME_ENCODER_H
#define __GLME_ENCODER_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifndef __INLINE__
#define __INLINE__ extern inline
#endif

/**
 * Gob Like Message encoder structure
 */
typedef struct glme_encoder {
  char *buf;
  size_t buflen;	// size of buffer in bytes
  size_t count;		// number of bytes writen into the buffer (count <= buflen)
  int owner;
} glme_encoder_t;


/**
 * Initialize encoder.
 *
 * @param enc
 *    Encoder to initialize
 * @param len
 *    Size of the initial encoding buffer.
 *
 * @returns
 *    Pointer to initialized encoder. Null pointer of failure.
 */
__INLINE__
glme_encoder_t *glme_encoder_init(glme_encoder_t *enc, size_t len)
{
  if (enc) {
    enc->count = 0;
    enc->owner = 0;
    // allow initializing to zero size
    if (len > 0) {
      enc->buf = malloc(len);
      if (! enc->buf)
	return (glme_encoder_t *)0;
      enc->owner = 1;
    } else {
      enc->buf = (char *)0;
    }
    enc->buflen = enc->buf ? len : 0;
  }
  return enc;
}

/**
 * Initialize encoder from externally allocated space.
 *
 * @param enc
 *    Encoder to initialize
 * @param buf
 *    External buffer space
 * @param len
 *    Size of the initial encoding buffer.
 *
 * @returns
 *    Pointer to initialized encoder. Null pointer of failure.
 */
__INLINE__
glme_encoder_t *glme_encoder_make(glme_encoder_t *enc, char *buf, size_t len)
{
  if (enc) {
    enc->count = 0;
    enc->owner = 0;
    enc->buf = buf;
    enc->buflen = len;
  }
  return enc;
}

/**
 * Create a new encoder of given size.
 *
 * @param len
 *    Initial size of the encoding buffer.
 *
 * @returns
 *    New encoder or null on error.
 */
__INLINE__
glme_encoder_t *glme_encoder_new(size_t len)
{
  glme_encoder_t *enc = malloc(sizeof(glme_encoder_t));
  if (enc)
    return glme_encoder_init(enc, len);
  return (glme_encoder_t *)0;
}

/**
 * Close encoder. Release data buffers.
 *
 * @param enc
 *    The encoder
 */
__INLINE__
void glme_encoder_close(glme_encoder_t *enc)
{
  if (enc) {
    if (enc->buf && enc->owner)
      free(enc->buf);
    enc->buflen = 0;
    enc->count = 0;
  }
}

/**
 * Free space allocated to the encoder.
 */
__INLINE__
void glme_encoder_free(glme_encoder_t *enc)
{
  if (enc) {
    if (enc->buf && enc->owner)
      free(enc->buf);
    enc->buflen = 0;
    enc->count = 0;
    free(enc);
  }
}

/**
 * Reset the encoder write pointer.
 */
__INLINE__
void glme_encoder_reset(glme_encoder_t *enc)
{
  if (enc)
    enc->count = 0;
}


/**
 * Increase buffer size for the encoder.
 *
 * @param enc
 *    Encoder
 * @param increase
 *    Number of bytes to increase the buffer. New size will be
 *    current size + increase.
 */
__INLINE__
void glme_encoder_resize(glme_encoder_t *enc, size_t increase)
{
  char *b = malloc(enc->buflen + increase);
  if (b) {
    if (enc->buf) {
      if (enc->count > 0)
	memcpy(b, enc->buf, enc->count);
      free(enc->buf);
    }
    enc->buf = b;
    enc->buflen += increase;
  }
}

/**
 * Get pointer to the encoder's raw data buffer.
 */
__INLINE__
char *glme_encoder_data(glme_encoder_t *enc)
{
  return enc ? enc->buf : (char *)0;
}

/**
 * Get number of bytes writen into the encoder.
 */
__INLINE__
size_t glme_encoder_bytes(glme_encoder_t *enc)
{
  return (size_t)(enc ? enc->count : 0);
}

/**
 * Write encoded message to the given file descriptor.
 */
int glme_encoder_writem(glme_encoder_t *enc, int fd);


/**
 * Encode unsigned 64 bit integer into the specified encoder.
 *
 * @param enc
 *    Encoder to write data into.
 * @param v
 *    Data to encode
 *
 * @returns
 *    Number of bytes writen. If encoder buffer resize fails returns -1 for
 *    error.
 */
int glme_encode_uint64(glme_encoder_t *enc, uint64_t v);

/**
 * Encode signed integer into the specified encoder.
 *
 * @see glme_encode_uint64
 */
int glme_encode_int64(glme_encoder_t *enc, int64_t v);

/**
 * Encode double precision floating point number into the specified encoder.
 *
 * @see glme_encode_uint64
 */
int glme_encode_double(glme_encoder_t *enc, double v);

/**
 * Encode uninterpreted byte stream into the specified encoder.
 *
 * @see glme_encode_uint64
 */
int glme_encode_bytes(glme_encoder_t *enc, void *s, size_t len);


/**
 * Encode unsigned long into the specified encoder.
 *
 * @see glme_encode_uint64
 */
__INLINE__
int glme_encode_ulong(glme_encoder_t *enc, unsigned long v)
{
  return glme_encode_uint64(enc, (uint64_t)v);
}

/**
 * Encode signed long into the specified encoder.
 *
 * @see glme_encode_uint64
 */
__INLINE__
int glme_encode_long(glme_encoder_t *enc, long v)
{
  return glme_encode_uint64(enc, (int64_t)v);
}

/**
 * Encode unsigned int into the specified encoder.
 *
 * @see glme_encode_uint64
 */
__INLINE__
int glme_encode_uint(glme_encoder_t *enc, unsigned int v)
{
  return glme_encode_int64(enc, (uint64_t)v);
}

/**
 * Encode signed int into the specified encoder.
 *
 * @see glme_encode_uint64
 */
__INLINE__
int glme_encode_int(glme_encoder_t *enc, int v)
{
  return glme_encode_int64(enc, (int64_t)v);
}

/**
 * Encode single precision float into the specified encoder.
 *
 * @see glme_encode_uint64
 */
__INLINE__
int glme_encode_float(glme_encoder_t *enc, float v)
{
  return glme_encode_double(enc, (double)v);
}

/**
 * Encode string into the specified encoder.
 *
 * @see glme_encode_uint64
 */
__INLINE__
int glme_encode_string(glme_encoder_t *enc, char *s)
{
  return glme_encode_bytes(enc, s, strlen(s)+1);
}

/**
 * Encode array start into the specified encoder.
 *
 * @see glme_encode_uint64
 */
__INLINE__
int glme_encode_start_array(glme_encoder_t *enc, size_t sz)
{
  return glme_encode_uint64(enc, (uint64_t)sz);
}

__INLINE__
int glme_encode_end_array(glme_encoder_t *enc, size_t sz)
{
  return 0;
}

/**
 * Encode struct start into the specified encoder.
 *
 * @see glme_encode_uint64
 */
__INLINE__
int glme_encode_start_struct(glme_encoder_t *enc, int typeid)
{
  return glme_encode_int64(enc, (int64_t)typeid);
}

__INLINE__
int glme_encode_end_struct(glme_encoder_t *enc)
{
  return glme_encode_uint64(enc, (uint64_t)0);
}

#endif

// Local Variables:
// indent-tabs-mode: nil
// End:
