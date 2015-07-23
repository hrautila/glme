
// Copyright (c) Harri Rautila, 2015

// GLME is Gob Like Message

#ifndef __GLME_H
#define __GLME_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifndef __INLINE__
#define __INLINE__ extern inline
#endif

#ifdef __cplusplus
extern "C" {
#endif

  enum glme_types {
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
    GLME_BASE_MAX       = 15
  };

  enum glme_flags {
    GLME_F_NONE   = 0x0,
    GLME_F_ARRAY  = 0x1,
    GLME_F_PTR    = 0x2
  };

  /* Not yet used, needs some thought. */
  enum glme_errors {
    GLME_E_INPUT = -1,
    GLME_E_INVAL = -2,
    GLME_E_TYPE  = -3
  };

/**
 * Gob Like Message Encoding buffer
 */
typedef struct glme_buf_s {
  char *buf;
  size_t buflen;	// size of buffer in bytes
  size_t count;		// number of bytes writen into the buffer (count <= buflen)
  size_t current;	// current read pointer (current <= count <= buflen)
  int owner;
  void *user;           // Pointer to user context for encoding/decoding functions.
} glme_buf_t;


typedef int (*encoder)(glme_buf_t *, const void *);
typedef int (*decoder)(glme_buf_t *, void *);

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
extern glme_buf_t *glme_buf_init(glme_buf_t *gbuf, size_t len);

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
extern glme_buf_t *glme_buf_make(glme_buf_t *gbuf, char *data, size_t len, size_t count);

/**
 * Create new glme_buf with buffer space of len bytes.
 *
 * @param len
 *   Requested initial buffer space in bytes.
 *
 * @return
 *   New initialized glme_buf.
 */
extern glme_buf_t *glme_buf_new(size_t len);

/**
 * Close the glme_buf. Releases allocated buffer and reset read pointers.
 */
extern void glme_buf_close(glme_buf_t *gbuf);

/**
 * Release the glme_buf.
 */
extern void glme_buf_free(glme_buf_t *gbuf);

/**
 * Reset glme_buf read pointer.
 */
extern void glme_buf_reset(glme_buf_t *gbuf);

/**
 * Get glme_buf read pointer.
 */
extern size_t glme_buf_at(glme_buf_t *gbuf);

/**
 * Set glme_buf read pointer.
 */
extern void glme_buf_seek(glme_buf_t *gbuf, size_t pos);

/**
 * Pushback read pointer.
 */
extern void glme_buf_pushback(glme_buf_t *gbuf, size_t n);

/**
 * Clear glme_buf contents.
 */
extern void glme_buf_clear(glme_buf_t *gbuf);

/**
 * Get content
 */
extern char *glme_buf_data(glme_buf_t *gbuf);

/**
 * Get encoded content length.
 */
extern size_t glme_buf_len(glme_buf_t *gbuf);

/**
 * Get size.
 */
extern size_t glme_buf_size(glme_buf_t *gbuf);

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
 * Disown glme_buf allocated space.
 */
extern void glme_buf_disown(glme_buf_t *gbuf);

/**
 * Own glme_buf allocated space.
 */
extern void glme_buf_own(glme_buf_t *gbuf);


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
                                  size_t len, size_t esize, encoder efunc);

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
                             size_t len, size_t esize, encoder efunc);
extern int glme_encode_value_array(glme_buf_t *enc, int typeid, const void *vptr,
                                   size_t len,  size_t esize, encoder efunc);

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
                             const void *vptr, size_t nlen, size_t esize, encoder efunc);

/**
 * Encode structure end mark into the specified buffer.
 */
extern int glme_encode_end_struct(glme_buf_t *gbuf);

/**
 * Encode structure into the specified buffer.
 */
extern int glme_encode_struct(glme_buf_t *enc, int typeid, const void *ptr, encoder efunc);

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
                             void *vptr, size_t *nlen, size_t esize, decoder dfunc);

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
 */
extern int glme_decode_struct(glme_buf_t *dec, int typeid, void *dptr, decoder dfunc);

/**
 * Read structure value from the specified buffer.
 */
extern int glme_decode_value_struct(glme_buf_t *dec, void *dptr, decoder dfunc);

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
                                  size_t len, size_t esize, decoder func);

/**
 * Read array from the specified buffer.
 */
extern int glme_decode_array(glme_buf_t *dec, int *typeid, void **dst,
                             size_t *len, size_t esize, decoder func);

/**
 * Read array value from the specified buffer. (Array sans ARRAY typeid)
 */
extern int glme_decode_value_array(glme_buf_t *dec, int *typeid, void **dst,
                                   size_t *len, size_t esize, decoder func);

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
#define GLME_ENCODE_FLD_INT(enc, elem, defval)                      \
  do {                                                              \
    int64_t __i64 = (int64_t)(elem);                                \
    __ne = (elem) != defval;                                        \
    __e = glme_encode_field(enc, &__delta, GLME_INT, 0, &__i64, 0,  \
                            __ne, (encoder)glme_encode_int64);      \
    if (__e < 0) return __e;                                        \
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
                            __ne, (encoder)glme_encode_uint64);     \
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
                            __ne, (encoder)glme_encode_double);      \
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
                            (elem), 0, 1, (encoder)0);     \
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
                            (elem), (len), __ne, (encoder)0); \
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
                              (encoder)vfunc);                  \
      if (__e < 0) return __e;                                  \
  } while (0)

#define GLME_ENCODE_FLD_INT_VECTOR(enc, elem, vfunc)            \
  do {								\
    __nl = sizeof(elem)/sizeof((elem)[0]);                      \
    __e = glme_encode_field(enc, &__delta,                      \
                            GLME_INT, GLME_F_ARRAY,             \
                            (elem), __nl, sizeof((elem)[0]),    \
                            (encoder)vfunc);                    \
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
                              (encoder)vfunc);                  \
      if (__e < 0) return __e;                                  \
  } while (0)

#define GLME_ENCODE_FLD_UINT_VECTOR(enc, elem, vfunc)           \
  do {								\
    __nl = sizeof(elem)/sizeof((elem)[0]);                      \
    __e = glme_encode_field(enc, &__delta,                      \
                            GLME_UINT, GLME_F_ARRAY,           \
                            (elem), __nl, sizeof((elem)[0]),    \
                            (encoder)vfunc);                    \
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
                              (encoder)vfunc);                  \
      if (__e < 0) return __e;                                  \
  } while (0)

#define GLME_ENCODE_FLD_FLOAT_VECTOR(enc, elem, vfunc)          \
  do {								\
    __nl = sizeof(elem)/sizeof((elem)[0]);                      \
    __e = glme_encode_field(enc, &__delta,                      \
                            GLME_FLOAT, GLME_F_ARRAY,           \
                            (elem), __nl, sizeof((elem)[0]),    \
                            (encoder)vfunc);                    \
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
  // empty expression after label             \
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
                            &__nl, 1, (decoder)glme_decode_int64); \
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
                            &__nl, 1, (decoder)glme_decode_uint64); \
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
                            &__nl, 1, (decoder)glme_decode_double); \
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
                          __ptr, &__nl, 1, (decoder)0);             \
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
                            &(elem), &__nl, 1, (decoder)0);         \
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
                            0, sizeof((elem)[0]), (decoder)func);       \
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
                            0, sizeof(elem), (decoder)func);    \
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
                            &(len), sizeof((elem)[0]), (decoder)func);  \
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
                            (decoder)func);                         \
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
                            &(len), sizeof((elem)[0]), (decoder)func);  \
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
                            (decoder)func);                         \
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
                            &(len), sizeof((elem)[0]), (decoder)func); \
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
                            (decoder)func);                            \
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
    // empty expression after label                  \
    do {} while (0)
    


#ifdef __cplusplus
}
#endif

#endif  // __GLME_H


// Local Variables:
// indent-tabs-mode: nil
// End:
