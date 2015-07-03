
// Copyrigth (c) Harri Rautila, 2015

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "glme/gobber.h"

#ifdef __INLINE__
#undef __INLINE__
#endif
#define __INLINE__

#include "glme/decoder.h"

/**
 * Read message from the given file descriptor.
 *
 * @param dec
 *    Decoder
 * @param fd
 *    Open file descriptor
 *
 * @returns
 *    Number of bytes read from the file descriptor.
 */
int glme_decoder_readm(glme_decoder_t *dec, int fd)
{
  int n, nc, rc = 0, dc;

  dc = dec->count;
 retry:
  if (dec->buflen - dec->count < 9) {  // 9 is max length of encoded int
    if (rc > 1)
      return -1;
    glme_decoder_resize(dec, dec->buflen < 1024 ? dec->buflen + 9 : 1024);
    rc++;
    goto retry;
  }
  
  // read one byte
  if ((n = read(fd, &dec->buf[dc], 1)) < 0)
    return -1;

 reread:
  nc = 0;
  dec->count += n;
  n = gob_decode_uint(&nc, &dec->buf[dc], dec->count - dc);
  if (n > dec->count - dc) {
    // underflow; read more 
    //printf("..readm: read %ld bytes more\n", n - (dec->count - dc));
    n = read(fd, &dec->buf[dec->count], n - (dec->count - dc));
    if (n < 0)
      return -1;
    goto reread;
  } 

  // here have message length in nc; read it in
  rc = 0;
  while (nc > dec->buflen - dec->count ) {
    if (rc > 1)
      return -1;
    // need more space; resize
    glme_decoder_resize(dec, nc - (dec->buflen - dec->count));
    rc++;
  }

  n = read(fd, &dec->buf[dec->count], nc);
  dec->count += n;
  return dec->count - dc;
}


int glme_decode_uint64(glme_decoder_t *dec, uint64_t *v)
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

int glme_decode_uint64_peek(glme_decoder_t *dec, uint64_t *v)
{
  int n;

  n = gob_decode_uint64(v, &dec->buf[dec->current], dec->count-dec->current); 
  if (n > dec->count - dec->current) {
    // under flow
    return -n;
  }
  return n;
}

int glme_decode_int64(glme_decoder_t *dec, int64_t *v)
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

int glme_decode_int64_peek(glme_decoder_t *dec, int64_t *v)
{
  int n;
  n = gob_decode_int64(v, &dec->buf[dec->current], dec->count-dec->current); 
  if (n > dec->count - dec->current) {
    // under flow
    return -n;
  }
  return n;
}

int glme_decode_double(glme_decoder_t *dec, double *v)
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

int glme_decode_vec(glme_decoder_t *dec, void *s, size_t len)
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

int glme_decode_bytes(glme_decoder_t *dec, void **s)
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

int glme_decode_string(glme_decoder_t *dec, char **s)
{
  return glme_decode_bytes(dec, (void **)s);
}

int glme_decode_end_struct(glme_decoder_t *dec)
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
