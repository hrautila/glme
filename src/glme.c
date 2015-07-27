
// Copyright (c) Harri Rautila, 2015


#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef __GLME_INLINE__
#undef __GLME_INLINE__
#endif

// define as empty
#define __GLME_INLINE__

#include "glme.h"

/*
 * We assume that number of encoder/decoder specs is relatively small and therefore
 * use simple linear search. Alternatively could sort them in increasing order and
 * use binary search.
 */


void glme_base_init(glme_base_t *base, glme_spec_t *specs, unsigned int nelem,
                    glme_allocator_t *alloc)
{
  base->nelem = base->owner = 0;
  if (specs) {
    base->handlers = specs;
    base->nelem = nelem;
  } else if (nelem > 0) {
    base->handlers = (glme_spec_t *)calloc(nelem, sizeof(glme_spec_t));
    if (base->handlers) {
      base->nelem = nelem;
      base->owner = 1;
    }
  }
  base->malloc  = alloc && alloc->malloc  ? alloc->malloc  : malloc;
  base->free    = alloc && alloc->free    ? alloc->free    : free;
  base->realloc = alloc && alloc->realloc ? alloc->realloc : realloc;
  base->calloc  = alloc && alloc->calloc  ? alloc->calloc  : calloc;
}

glme_spec_t *glme_base_find(glme_base_t *base, int typeid)
{
  int i;
  if (!base)
    return (glme_spec_t *)0;
  for (i = 0; i < base->nelem; i++) {
    if (typeid == base->handlers[i].typeid)
      return &base->handlers[i];
  }
  return (glme_spec_t *)0;
}

int glme_base_register(glme_base_t *base, glme_spec_t *spec)
{
  int i;
  if (!base)
    return -1;

  for (i = 0; i < base->nelem; i++) {
    if (base->handlers[i].typeid == 0) {
      base->handlers[i] = *spec;
      return i;
    }
  }
  return -1;
}

void glme_base_unregister(glme_base_t *base, int typeid)
{
  glme_spec_t *spec = glme_base_find(base, typeid);
  if (spec)
    spec->typeid = 0;
}


size_t glme_buf_resize(glme_buf_t *gbuf, size_t increase)
{
  // resize only of owner of the data buffer or if current size is zero
  // and owner is not set
  if (gbuf->owner == 1 || gbuf->buflen == 0) {
    char *b = glme_realloc(gbuf, gbuf->buf, gbuf->buflen + increase);
    if (b) {
      gbuf->buf = b;
      gbuf->buflen += increase;
      gbuf->owner = 1;
      return  gbuf->buflen;
    }
    gbuf->last_error = GLME_E_NOMEM;
  }
  return 0;
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
