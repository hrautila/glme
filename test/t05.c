

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "glme.h"

// Bytes vectors and strings with encoded type information.

main(int argc, char *argv)
{
  glme_buf_t gbuf;
  char vec[5], *s;
  int n0, n1;

  glme_buf_init(&gbuf, 1024);

  glme_encode_vector(&gbuf, "hello", 5);
  glme_encode_string(&gbuf, "hello");

  if (argc > 1)
    write(1, glme_buf_data(&gbuf), glme_buf_len(&gbuf));

  n0 = glme_decode_vector(&gbuf, vec, 5);
  n1 = glme_decode_string(&gbuf, &s);
  
  assert(memcmp(vec, "hello", 5) == 0 && n0 == 7);
  assert(strcmp(s, "hello") == 0 && n1 == 8);
  return 0;
}

/* Local Variables:
 * indent-tabs-mode: nil
 * End:
 */
