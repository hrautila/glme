

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "glme.h"

// Byte vectors and array values.

main(int argc, char *argv)
{
  glme_buf_t gbuf;
  char vec[5], *s, *ptr;
  int n0, n1;

  glme_buf_init(&gbuf, 1024);

  glme_encode_bytes(&gbuf, "hello", 5);
  glme_encode_bytes(&gbuf, "hello", 6);

  if (argc > 1)
    write(1, glme_buf_data(&gbuf), glme_buf_len(&gbuf));

  ptr = &vec[0];
  n0 = glme_decode_bytes(&gbuf, (void **)&ptr, 4);
  n1 = glme_decode_bytes(&gbuf, (void **)&s, 0);
  
  assert(memcmp(vec, "hell", 4) == 0 && n0 == 6);
  assert(strcmp(s, "hello") == 0 && n1 == 7);
  return 0;
}

/* Local Variables:
 * indent-tabs-mode: nil
 * End:
 */
