
// Copyright (c) Harri Rautila, 2015

#ifndef _GOBBER_H
#define _GOBBER_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef __INLINE__
#define __INLINE__ extern inline
#endif


/**
 * Encodes unsigned 64bit integer into the specified buffer.
 *
 * Go gob package: "An unsigned integer is sent one of two
 * ways. If it is less than 128, it is sent as a byte with that value.
 * Otherwise it is sent as a minimal-length big-endian (high byte first) byte
 * stream holding the value, preceded by one byte holding the byte count,
 * negated. Thus 0 is transmitted as (00), 7 is transmitted as (07) and 256
 * is transmitted as (FE 01 00). "
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
extern int gob_encode_uint64(char *buf, size_t buf_size, uint64_t uv);

/**
 * Encodes signed 64 bit integer into the specified buffer.
 *
 * From gob package: "A signed integer, i, is encoded within an unsigned integer, u.
 * Within u, bits 1 upward contain the value; bit 0 says whether they should be
 * complemented upon receipt."
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
extern int gob_encode_int64(char *buf, size_t buf_size, int64_t v);

/**
 * Encode uninterpreted byte stream into the specified buffer.
 *
 * @param buf
 *   The buffer into which to encode the parameter. The pointer must point
 *   to "empty" space in the buffer.
 * @param buf_size
 *   The number of bytes available.
 * @param s
 *   The byte stream to encode.
 * @param len
 *   Length of the byte stream.
 *
 * @return
 *   The number of bytes written by the encode operation. A return value greater
 *   than buf_size indicates error (buffer overflow) and no bytes are written.
 */
extern int gob_encode_bytes(char *buf, size_t buf_size, void *s, size_t len);

/**
 * Encode double precision number into the specified buffer.
 *
 * From the gob packate: "Floating-point numbers are always sent as a representation of
 * a 64 bit IEEE value. That value is converted to a uint64. The uint64 is then
 * byte-reversed and sent as a regular unsigned integer. The byte-reversal means the
 * exponent and high-precision part of the mantissa go first. Since the low bits are
 * often zero, this can save encoding bytes. For instance, 17.0 is encoded in only
 * three bytes (FE 31 40)."
 * 
 * @see gob_encode_uint64
 */
extern int gob_encode_double(char *buf, size_t buf_size, double v);

/**
 * Encode type identifier into the specified buffer.
 *
 * @see gob_encode_int64
 */
extern int gob_encode_type(char *buf, size_t buf_size, int id);

/**
 * Write end of struct mark to the specified buffer.
 */
extern int gob_encode_struct_end(char *buf, size_t buf_size);

/**
 * Extract unsigned 64 bit integer from the specified buffer.
 *
 * @param ull
 *   Pointer to the unsigned 64 bit integer where result is stored.
 * @param buf
 *   Source buffer.
 * @param buf_size
 *   Number of bytes available in the buffer.
 *
 * @returns
 *   Number of bytes consumed from the source buffer. If return value
 *   greater than the number of available buffer (buf_size) then partial
 *   decode happened.
 */
extern int gob_decode_uint64(uint64_t *ull, char *buf, size_t buf_size);

/**
 */
extern int gob_decode_int64(int64_t *v, char *buf, size_t buf_size);

/**
 */
extern int gob_decode_double(double *v, char *buf, size_t buf_size);

/**
 */
extern int gob_decode_bytes(void *v, size_t vlen, char *buf, size_t buf_size);

// convinience functions

/**
 * Encode unsigned long into the specified buffer.
 */
extern int gob_encode_ulong(char *buf, size_t buf_size, unsigned long v);

/**
 * Encode long into the specified buffer.
 */
extern int gob_encode_long(char *buf, size_t buf_size, long v);

/**
 * Encode unsigned int into the specified buffer.
 */
extern int gob_encode_uint(char *buf, size_t buf_size, unsigned int v);

/**
 * Encode int into the specified buffer.
 */
extern int gob_encode_int(char *buf, size_t buf_size, int v);

/**
 * Encode boolean into the specified buffer.
 */
extern int gob_encode_bool(char *buf, size_t buf_size, int v);

/**
 * Encode single precision real number into the specified buffer.
 */
extern int gob_encode_float(char *buf, size_t buf_size, float v);

/**
 * Encode string into the specified buffer.
 */
extern int gob_encode_string(char *buf, size_t buf_size, char *s);


/**
 * Decode unsigned long from the specified buffer.
 */
extern int gob_decode_ulong(unsigned long *v, char *buf, size_t buf_size);

/**
 * Decode unsigned long from the specified buffer.
 */
extern int gob_decode_long(long *v, char *buf, size_t buf_size);

/**
 * Decode unsigned long from the specified buffer.
 */
extern int gob_decode_uint(unsigned int *v, char *buf, size_t buf_size);

/**
 * Decode unsigned long from the specified buffer.
 */
extern int gob_decode_int(int *v, char *buf, size_t buf_size);

/**
 * Decode single precision float from the specified buffer.
 */
extern int gob_decode_float(float *v, char *buf, size_t buf_size);

#endif

// Local Variables:
// indent-tabs-mode: nil
// End:
