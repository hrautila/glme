

#include <stdio.h>
#include <assert.h>
#include "glme.h"

// Error in simple typed variable decoding

main(int argc, char *argv)
{
  glme_buf_t gbuf;
  int k0;
  int n0, n1, a1, a = 1; double b1, b = -2.0;

  glme_buf_init(&gbuf, 1024);
  glme_encode_int(&gbuf, &a);
  glme_encode_double(&gbuf, &b);

  if (argc > 1)
    write(1, glme_buf_data(&gbuf), glme_buf_len(&gbuf));

  n0 = glme_decode_int(&gbuf, &a1);
  n1 = glme_decode_int(&gbuf, (int *)&b1);
  
  assert(a == a1 && n0 == 2);
  assert(b != b1 && n1 < 0);
  return 0;
}

/* Local Variables:
 * indent-tabs-mode: nil
 * End:
 */
