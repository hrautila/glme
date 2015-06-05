
// Copyright (c) Harri Rautila, 2015

#ifndef __GLME_H
#define __GLME_H

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>

#include <glme/encoder.h>
#include <glme/decoder.h>


/**
 * Encode function stardard definitions
 */
#define GLME_ENCODE_STDDEF                      \
  int __n, __nc = 0, __delta = 1, __skip_array

/**
 * Encode function stardard ending
 */
#define GLME_ENCODE_END				\
  __n = glme_encode_end_struct(enc);		\
  if (__n < 0) return -4;                       \
  __nc += __n;					\
  return __nc

/**
 * Encode structure type id
 */
#define GLME_ENCODE_TYPE(enc, typeid)                   \
  do {                                                  \
    __n = glme_encode_start_struct(enc, typeid);	\
    if (__n < 0) return -1;                             \
    __nc += __n;					\
    __delta = 1;					\
  } while (0);

/**
 * Initialize field delta.
 */
#define GLME_ENCODE_DELTA(enc)

#define GLME_ENCODE(enc, fno, elem, typename, nullv)	\
  do {							\
    if ( (elem) != nullv ) {				\
      __n = glme_encode_uint64(enc, __delta);		\
      if (__n < 0) return -(fno+10);                    \
      __nc += __n;                                      \
      __n = glme_encode_ ## typename (enc, elem);       \
      if (__n < 0) return -(fno+10);                    \
      __nc += __n;					\
      __delta = 1;					\
    } else {						\
      __delta += 1;					\
    }							\
  } while (0)


/**
 * Encode fixed length byte vector.
 */
#define GLME_ENCODE_VEC(enc, fno, vec, len)	\
  do {						\
    if ( (len) != 0 ) {				\
      __n = glme_encode_uint64(enc, __delta);	\
      if (__n < 0) return -(fno+10);                    \
      __nc += __n;                              \
      __n = glme_encode_bytes(enc, vec, len);	\
      if (__n < 0) return -(fno+10);                    \
      __nc += __n;                              \
      __delta = 1;				\
    } else {					\
      __delta += 1;				\
    }						\
  } while (0)

/**
 * Encode variable length string.
 */
#define GLME_ENCODE_STR(enc, fno, vec)			\
  do {							\
    if ( (vec) && strlen(vec) != 0 ) {			\
      __n = glme_encode_uint64(enc, __delta);		\
      if (__n < 0) return -(fno+10);                    \
      __nc += __n;                                      \
      __n = glme_encode_bytes(enc, vec, strlen(vec)+1);	\
      if (__n < 0) return -(fno+10);                    \
      __nc += __n;                                      \
      __delta = 1;					\
    } else {						\
      __delta += 1;					\
    }							\
  } while (0)

/**
 * Encode fixed length array
 */
#define GLME_ENCODE_ARRAY(enc, fno, vec, len, typename)	\
  do {							\
    if ( (len) != 0 ) {					\
      int __k;						\
      __n = glme_encode_uint64(enc, __delta);		\
      if (__n < 0) return -(fno+10);                    \
      __nc += __n;                                      \
      __n = glme_encode_uint64(enc, len);               \
      if (__n < 0) return -(fno+10);                    \
      __nc += __n;                                      \
      for (__k = 0; __k < (len); __k++) {               \
	__n = glme_encode_ ## typename (enc, vec[__k]);	\
        if (__n < 0) return -(fno+10);                  \
        __nc += __n;					\
      }							\
      __delta = 1;					\
    } else {						\
      __delta += 1;					\
    }							\
  } while (0)

/**
 * Encode start of array; encode current delta and array length
 *
 * Code for encoding actual elements must follow this definition.
 */
#define GLME_ENCODE_START_ARRAY(enc, fno, len)  \
  do {                                          \
    __skip_array = 0;                           \
    if ( (len) == 0) {                          \
      __skip_array = 1;                         \
      goto empty_array ## fno;                  \
    }                                           \
    __n = glme_encode_uint64(enc, __delta);     \
    if (__n < 0) return -(fno+10);              \
    __nc += __n;                                \
    __n = glme_encode_uint64(enc, len);         \
    if (__n < 0) return -(fno+10);              \
    __nc += __n;                                \
    __delta = 1;                                \
  } while (0)


/**
 * End of array for matching start of array; inserts code for handling
 * zero length arrays.
 */
#define GLME_ENCODE_END_ARRAY(enc, fno)		\
    empty_array##fno:				\
    do {                                        \
      if (__skip_array != 0) {			\
        __delta += 1;                           \
      }                                         \
    } while (0)
  
  

/**
 * Encode embedded struct
 */
#define GLME_ENCODE_STRUCT(enc, fno, elem, func)	\
    do {                                                \
      int __k;						\
      __n = glme_encode_uint64(enc, __delta);		\
      if (__n < 0) return -(fno+10);                    \
      __nc += __n;                                      \
      __n = (*func) (enc, elem);                        \
      if (__n < 0) return -(fno+10);                    \
      __nc += __n;					\
      __delta = 1;					\
    } while (0)

/**
 * Encode struct pointer
 */
#define GLME_ENCODE_STRUCT_PTR(enc, fno, elem, func)	\
  do {							\
    if ( (elem) ) {					\
      __n = glme_encode_uint64(enc, __delta);		\
      if (__n < 0) return -(fno+10);                    \
      __nc += __n;                                      \
      __n = (*func) (enc, elem);                        \
      if (__n < 0) return -(fno+10);                    \
      __nc += __n;					\
      __delta = 1;					\
    } else {						\
      __delta += 1;					\
    }							\
  } while (0)

/**
 * Encode unsigned 64 bit integer.
 */
#define GLME_ENCODE_UINT64(enc, fno, elem)	\
    GLME_ENCODE(enc, fno, elem, uint64, 0)
  
/**
 * Encode signed 64 bit integer.
 */
#define GLME_ENCODE_INT64(enc, fno, elem)	\
  GLME_ENCODE(enc, fno, elem, int64, 0)

/**
 * Encode unsigned long integer.
 */
#define GLME_ENCODE_ULONG(enc, fno, elem)	\
    GLME_ENCODE(enc, fno, elem, ulong, 0)
  
/**
 * Encode signed long integer.
 */
#define GLME_ENCODE_LONG(enc, fno, elem)	\
  GLME_ENCODE(enc, fno, elem, long, 0)

/**
 * Encode unsigned int.
 */
#define GLME_ENCODE_UINT(enc, fno, elem)	\
    GLME_ENCODE(enc, fno, elem, uint, 0)
  
/**
 * Encode signed integer.
 */
#define GLME_ENCODE_INT(enc, fno, elem)	\
  GLME_ENCODE(enc, fno, elem, int, 0)


/**
 * Encode double precision 64 bit floating point number.
 */
#define GLME_ENCODE_DOUBLE(enc, fno, elem)	\
    GLME_ENCODE(enc, fno, elem, double, 0.0)

/**
 * Encode single precision 32 bit floating point number.
 */
#define GLME_ENCODE_FLOAT(enc, fno, elem)	\
    GLME_ENCODE(enc, fno, elem, float, 0.0)


// ----------------------------------------------------------------------------
// decode macros

/**
 * Decode function stardard definitions
 */
#define GLME_DECODE_STDDEF			\
    int __n, __nc = 0;				\
    uint64_t __delta;				\
    int64_t __msgtype

/**
 * Decode function stardard ending
 */
#define GLME_DECODE_END				\
    if (__delta != 0) return -4;                \
    return __nc


#define GLME_DECODE_END_OLD                     \
    ready:					\
    __n = glme_decode_end_struct(dec);		\
    if (__n < 0) return -4;			\
    __nc += __n;				\
    return __nc

#define GLME_DECODE(dec, fno, elem, typename, nullv)	\
    do {						\
      if (__delta == 1) {				\
	__n = glme_decode_ ## typename(dec, elem);	\
	  if (__n < 0) return -(fno+10);		\
	  __nc += __n;					\
	  __n = glme_decode_uint64(dec, &__delta);	\
	  if (__n < 0) return -(fno+10);		\
	  __nc += __n;					\
      } else {						\
	*(elem) = nullv;				\
	if (__delta > 0 ) __delta -= 1;                 \
      }							\
    } while (0)

/**
 * Decode fixed size byte vector;
 */
#define GLME_DECODE_VEC(dec, fno, vec, len)		\
    do {                                                \
        if (__delta == 1) {				\
          __n = glme_decode_bytes(dec, vec, len);       \
          if (__n < 0) return -(fno+10);                \
          __nc += __n;					\
          __n = glme_decode_uint64(dec, &__delta);	\
          if (__n < 0) return -(fno+10);                \
          __nc += __n;					\
        } else {                                        \
          memset(vec, 0, len);				\
          if (__delta > 0 ) __delta -= 1;               \
        }                                               \
    } while (0)


/**
 * Decode string; 
 */
#define GLME_DECODE_STR(dec, fno, vec)			\
    do {						\
      if (__delta == 1) {				\
	__n = glme_decode_string(dec, vec);		\
	if (__n < 0) return -(fno+10);			\
	__nc += __n;					\
	__n = glme_decode_uint64(dec, &__delta);	\
	if (__n < 0) return -(fno+10);			\
	__nc += __n;					\
      } else {						\
	*(vec) = (char *)0;				\
	if (__delta > 0) __delta -= 1;			\
      }							\
    } while (0)


/**
 * Decode and verify type id.
 */
#define GLME_DECODE_TYPE(dec, typeid)		\
    do {					\
      __n = glme_decode_int64(dec, &__msgtype);	\
      if (__n < 0) return -1;			\
      if (__msgtype != typeid) return -2;	\
      __nc += __n;				\
    } while (0)

/**
 * Setup initial value for field delta.
 */
#define GLME_DECODE_DELTA(dec)                \
  do {                                        \
    __n = glme_decode_uint64(dec, &__delta);  \
    if (__n < 0) return -3;                   \
    __nc += __n;                              \
  } while (0)



#define GLME_DECODE_ARRAY(dec, fno, vec, len, typename)         \
    do {                                                        \
      if (__delta == 1) {                                       \
        int __k;                                                \
        uint64_t __alen;                                        \
        __n = glme_decode_uint64(dec, &__alen);                 \
        if (__n < 0) return -(fno+10);                          \
        __nc += __n;                                            \
        for (__k = 0; __k < len; __k++) {                       \
          __n = glme_decode_ ## typename(dec, &(vec)[__k]);     \
          if (__n < 0) return -(fno+10);                        \
          __nc += __n;                                          \
        }                                                       \
	__n = glme_decode_uint64(dec, &__delta);                \
	if (__n < 0) return -(fno+10);	                        \
	__nc += __n;				                \
      } else {                                                  \
        memset((vec), 0, (len)*sizeof(*(vec)));                 \
        if (__delta > 0) __delta -= 1;                          \
      }                                                         \
    } while (0)


/**
 * Expand code to deocode variable size array.
 *
 * @param dec
 *   Decoder
 * @param fno
 *   Field number
 * @param vec
 *   Array pointer
 * @param
 *   Variable to receive array length
 * @param typename
 *   Element typename (known GLME basic decoders)
 */
#define GLME_DECODE_VAR_ARRAY(dec, fno, vec, len, typename)     \
    do {                                                        \
      if (__delta == 1) {                                       \
        int __k;                                                \
        uint64_t __alen;                                        \
        typename *__vptr;                                       \
        __n = glme_decode_uint64(dec, &__alen);                 \
        if (__n < 0) return -(fno+10);                          \
        __nc += __n;                                            \
        __vptr = (typename *)malloc(__alen*sizeof(*(vec)));     \
        if (! __vptr) return -(fno+10);                         \
        for (__k = 0; __k < __alen; __k++) {                    \
          __n = glme_decode_ ## typename(dec, &__vptr[__k]);    \
          if (__n < 0) {free(__vptr); return -(fno+10);}        \
          __nc += __n;                                          \
        }                                                       \
	__n = glme_decode_uint64(dec, &__delta);                \
	if (__n < 0) {free(__vptr); return -(fno+10);}          \
        vec = __vptr;                                           \
        len = __alen;                                           \
	__nc += __n;				                \
      } else {                                                  \
        memset((vec), 0, (len)*sizeof(typename));               \
        if (__delta > 0) __delta -= 1;                          \
      }                                                         \
    } while (0)


/**
 * Decode embedded struct
 */
#define GLME_DECODE_STRUCT(dec, fno, elem, func)	\
   do {                                                 \
      if (__delta == 1) {                               \
        __n = (*func) (dec, elem);                      \
        if (__n < 0) return -(fno+10);                  \
        __nc += __n;					\
        __n = glme_decode_uint64(dec, &__delta);        \
        if (__n < 0) return -(fno+10);                  \
        __nc += __n;                                    \
      } else {                                          \
        memset(elem, 0, sizeof(*(elem)));               \
        if (__delta > 0) __delta -= 1;                  \
      }                                                 \
   } while (0)

/**
 * Decode struct pointer;
 *
 * @param dec
 *   Decoder
 * @param fno
 *   Field number
 * @param elem
 *   Field element
 * @param func
 *   Decoder function for struct type;
 * @param stype
 *   C type name for structure type
 */
#define GLME_DECODE_STRUCT_PTR(dec, fno, elem, func, stype)	\
      do {                                                      \
          if (__delta == 1) {                                   \
            stype *__sptr = (stype *)malloc(sizeof(stype));     \
            if (! __sptr) return -(fno+10);                     \
            __n = (*func) (dec, __sptr);                        \
            if (__n < 0) { free(__sptr); return -(fno+10);}     \
            __nc += __n;					\
            __n = glme_decode_uint64(dec, &__delta);            \
            if (__n < 0) { free(__sptr); return -(fno+10);}     \
            __nc += __n;                                        \
            elem = __sptr;                                      \
          } else {                                              \
            elem = (stype *)0;                                  \
            if (__delta > 0) __delta -= 1;                      \
          }                                                     \
      } while (0)

/**
 * Decode unsigned 64 bit integer
 */
#define GLME_DECODE_UINT64(dec, fno, elem)      \
    GLME_DECODE(dec, fno, elem, uint64, 0)

/**
 * Decode signed 64 bit integer
 */
#define GLME_DECODE_INT64(dec, fno, elem)       \
      GLME_DECODE(dec, fno, elem, int64, 0)

/**
 * Decode unsigned long integer
 */
#define GLME_DECODE_ULONG(dec, fno, elem)      \
    GLME_DECODE(dec, fno, elem, ulong, 0)

/**
 * Decode signed long integer
 */
#define GLME_DECODE_LONG(dec, fno, elem)       \
      GLME_DECODE(dec, fno, elem, long, 0)

/**
 * Decode unsigned integer
 */
#define GLME_DECODE_UINT(dec, fno, elem)      \
    GLME_DECODE(dec, fno, elem, uint, 0)

/**
 * Decode signed integer
 */
#define GLME_DECODE_INT(dec, fno, elem)       \
      GLME_DECODE(dec, fno, elem, int, 0)


/**
 * Decode double precision 64 bit floating point number
 */
#define GLME_DECODE_DOUBLE(dec, fno, elem)      \
      GLME_DECODE(dec, fno, elem, double, 0.0)

/**
 * Decode single precision 64 bit floating point number
 */
#define GLME_DECODE_FLOAT(dec, fno, elem)      \
      GLME_DECODE(dec, fno, elem, float, 0.0)


/**
 * Copy encoder data to decoder.
 *
 * @return
 *   Non zero for error. Zero for success.
 */
extern int glme_copy_to_decoder(glme_decoder_t *dec, glme_encoder_t *enc);
  
#endif // __GLME_H

// Local Variables:
// indent-tabs-mode: nil
// End:
