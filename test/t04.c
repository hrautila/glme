

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include "glme.h"

// Simple untyped values, improper decoding but no error.

main(int argc, char *argv)
{
  glme_buf_t gbuf;
  int n0, n1, a0 = 1, a1; int64_t b1;
  double b0 = -2.0;

  glme_buf_init(&gbuf, 1024);
  glme_encode_value_int(&gbuf, &a0);
  glme_encode_value_double(&gbuf, &b0);

  write(1, glme_buf_data(&gbuf), glme_buf_len(&gbuf));

  n0 = glme_decode_value_int(&gbuf, &a1);
  n1 = glme_decode_value_int64(&gbuf, &b1);
  
  assert(a1 == 1 && n0 == 1);
  assert( (double)b1 != -2.0 && n1 == 2);
  return 0;
}

/* Local Variables:
 * indent-tabs-mode: nil
 * End:
 */
