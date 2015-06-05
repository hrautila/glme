
#include <string.h>

#include "glme/glme.h"

int glme_copy_to_decoder(glme_decoder_t *dec, glme_encoder_t *enc)
{
  if (! (dec && enc) )
    return -1;

  if (dec->buflen < enc->count) {
    char *b = malloc(enc->count+64);
    if (! b) {
      return -1;
    }
    if (dec->buf)
      free(dec->buf);
    dec->buf = b;
    dec->buflen = enc->count+64;
  }

  memcpy(dec->buf, enc->buf, enc->count);
  dec->count = enc->count;
  dec->current = 0;
  return 0;
}


// Local Variables:
// indent-tabs-mode: nil
// End:
