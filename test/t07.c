

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "glme.h"

// Integer arrays, no size checking

main(int argc, char *argv)
{
  glme_buf_t gbuf;
  int ovec[5], ivec[5] = {1, 2, 3, 4, 5};
  int i, n0, n1, typeid;
  size_t len;

  glme_buf_init(&gbuf, 1024);

  glme_encode_array_start(&gbuf, GLME_INT, 5);
  for (i = 0; i < 5; i++)
    glme_encode_value_int(&gbuf, &ivec[i]);

  if (argc > 1)
    write(1, glme_buf_data(&gbuf), glme_buf_len(&gbuf));

  n0 = glme_decode_array_start(&gbuf, &typeid, &len);
  for (i = 0; i < len; i++) {
    glme_decode_value_int(&gbuf, &ovec[i]);
  }

  assert(typeid == GLME_INT);
  assert(len == 5);
  assert(memcmp(ivec, ovec, sizeof(ivec)) == 0);
  return 0;
}

/* Local Variables:
 * indent-tabs-mode: nil
 * End:
 */
