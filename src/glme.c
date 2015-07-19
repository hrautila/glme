
// Copyright (c) Harri Rautila, 2015


#include <string.h>
#include <stdlib.h>
#include <stdint.h>


#include "glme.h"

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
  }
  return gbuf;
}


glme_buf_t *glme_buf_make(glme_buf_t *gbuf, char *data, size_t len, size_t count)
{
  gbuf->buf = data;
  gbuf->buflen = len;
  gbuf->count = count;
  gbuf->current = 0;
  gbuf->owner = 0;
  return gbuf;
}


glme_buf_t *glme_buf_new(size_t len)
{
  glme_buf_t *gbuf = malloc(sizeof(glme_buf_t));
  if (gbuf)
    return glme_buf_init(gbuf, len);
  return (glme_buf_t *)0;
}

void glme_buf_close(glme_buf_t *gbuf)
{
  if (gbuf) {
    if (gbuf->buf && gbuf->owner)
      free(gbuf->buf);
    gbuf->buf = (char *)0;
    gbuf->buflen = 0;
    gbuf->count = 0;
    gbuf->current = 0;
  }
}

void glme_buf_free(glme_buf_t *gbuf)
{
  if (gbuf) {
    if (gbuf->buf && gbuf->owner)
      free(gbuf->buf);
    gbuf->buflen = 0;
    gbuf->count = 0;
    gbuf->current = 0;
    free(gbuf);
  }
}

void glme_buf_reset(glme_buf_t *gbuf)
{
  if (gbuf)
    gbuf->current = 0;
}

size_t glme_buf_at(glme_buf_t *gbuf)
{
  return gbuf ? gbuf->current : 0;
}

void glme_buf_seek(glme_buf_t *gbuf, size_t pos)
{
  if (gbuf)  
    gbuf->current = pos < gbuf->count ? pos : gbuf->count;
}

void glme_buf_pushback(glme_buf_t *gbuf, size_t n)
{
  if (!gbuf)
    return;
  gbuf->current -= n > gbuf->current ? gbuf->current : n;
}

void glme_buf_clear(glme_buf_t *gbuf)
{
  if (gbuf) {
    gbuf->count = gbuf->current = 0;
  }
}

char *glme_buf_data(glme_buf_t *gbuf)
{
  return gbuf ? gbuf->buf : (char *)0;
}

size_t glme_buf_len(glme_buf_t *gbuf)
{
  return gbuf ? gbuf->count : 0;
}

size_t glme_buf_size(glme_buf_t *gbuf)
{
  return gbuf ? gbuf->buflen : 0;
}


void glme_buf_resize(glme_buf_t *gbuf, size_t increase)
{
  // resize only of owner of the data buffer or if current size is zero
  // and owner is not set
  if (gbuf->owner == 1 || gbuf->buflen == 0) {
    char *b = realloc(gbuf->buf, gbuf->buflen + increase);
    if (b) {
      gbuf->buf = b;
      gbuf->buflen += increase;
      gbuf->owner = 1;
    }
  }
}

void glme_buf_disown(glme_buf_t *gbuf)
{
  if (gbuf)
    gbuf->owner = 0;
}

void glme_buf_own(glme_buf_t *gbuf)
{
  if (gbuf)
    gbuf->owner = 1;
}


int glme_buf_writem(glme_buf_t *enc, int fd)
{
  int n;
  char tmp[16];
  n = gob_encode_uint64(tmp, sizeof(tmp), enc->count);

  if (write(fd, tmp, n) < 0)
    return -1;
  if (write(fd, enc->buf, enc->count) < 0)
    return -1;
  
  return n + enc->count;
}

int glme_buf_readm(glme_buf_t *dec, int fd, size_t maxlen)
{
  int n, nc;
  uint64_t mlen;
  char tmp[12];
  glme_buf_t gbuf;

  if ((n = read(fd, tmp, 1)) < 0)
    return -1;
  if (n == 0)
    return 0;

  glme_buf_make(&gbuf, tmp, sizeof(tmp), 1);
  if ((n = glme_decode_value_uint64(&gbuf, &mlen)) < 0) {
    // read more; -n is length of encoded prefix; read missing part
    nc = -(n+1);
    n = read(fd, &tmp[1], nc);
    if (n < 0)
      return -1;
    if (n < nc)
      return -1;

    nc++;
    glme_buf_make(&gbuf, tmp, sizeof(tmp), nc);
    // decode again
    n = glme_decode_value_uint64(&gbuf, &mlen);
    if (n < 0) 
      return -1;
  }
  // here we have message length in mlen
  if (maxlen > 0 && mlen > maxlen)
    return -1;

  if (mlen > dec->buflen)
    glme_buf_resize(dec, mlen-dec->buflen);

  glme_buf_clear(dec);
  if (read(fd, dec->buf, mlen) < 0)
    return -1;

  dec->count = mlen;

  return (int)mlen + nc;
}



// Local Variables:
// indent-tabs-mode: nil
// End:
