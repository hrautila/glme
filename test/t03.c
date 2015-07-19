

#include <stdio.h>
#include <assert.h>
#include "glme.h"

// Simple untyped values

main(int argc, char *argv)
{
  glme_buf_t gbuf;
  int n0, n1, a1, a = 1; double b1, b = -2.0;

  glme_buf_init(&gbuf, 1024);
  glme_encode_value_int(&gbuf, &a);
  glme_encode_value_double(&gbuf, &b);

  if (argc > 1)
    write(1, glme_buf_data(&gbuf), glme_buf_len(&gbuf));

  n0 = glme_decode_value_int(&gbuf, &a1);
  n1 = glme_decode_value_double(&gbuf, &b1);
  
  assert(a1 == 1 && n0 == 1);
  assert(b1 == -2.0 && n1 == 2);
  return 0;
}

/* Local Variables:
 * indent-tabs-mode: nil
 * End:
 */
