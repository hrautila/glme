
// Copyright (c) Harri Rautila, 2015

#include <stdio.h>
#include <stdint.h>
#include "gobber.h"

typedef union decval_s {
  int64_t ival;
  double fval;
} decval_t;

int gobdec_int64(decval_t *ptr, char *buf, size_t len)
{
  return gob_decode_int64(&ptr->ival, buf, len);
}

int gobdec_uint64(decval_t *ptr, char *buf, size_t len)
{
  return gob_decode_uint64((uint64_t *)&ptr->ival, buf, len);
}

int gobdec_double(decval_t *ptr, char *buf, size_t len)
{
  return gob_decode_double(&ptr->fval, buf, len);
}
