
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "glme/glme.h"

#define MSG_DATA_ID 32
#define MSG_RECT_ID 33


typedef struct rect {
  int64_t x0;
  int64_t x1;
  int64_t y0;
  int64_t y1;
} rect_t;

int rectcmp(rect_t *a, rect_t *b)
{
  int k;
  if (a->x0 != b->x0)
    return a->x0 > b->x0 ? -1 : 1;
  if (a->x1 != b->x1)
    return a->x1 > b->x1 ? -1 : 1;
  if (a->y0 != b->y0)
    return a->y0 > b->y0 ? -1 : 1;
  if (a->y1 != b->y1)
    return a->y1 > b->y1 ? -1 : 1;
  return 0;
}

int glme_encode_rect_t(glme_encoder_t *enc, rect_t *rc)
{
  GLME_ENCODE_STDDEF;
  GLME_ENCODE_TYPE(enc, MSG_RECT_ID);
  GLME_ENCODE_DELTA(enc);
  GLME_ENCODE_INT64(enc, 0, rc->x0);
  GLME_ENCODE_INT64(enc, 1, rc->x1);
  GLME_ENCODE_INT64(enc, 2, rc->y0);
  GLME_ENCODE_INT64(enc, 3, rc->y1);
  GLME_ENCODE_END;
}

int glme_decode_rect_t(glme_decoder_t *dec, rect_t *rc)
{
  GLME_DECODE_STDDEF;
  GLME_DECODE_TYPE(dec, MSG_RECT_ID);
  GLME_DECODE_DELTA(dec);
  GLME_DECODE_INT64(dec, 0, &rc->x0);
  GLME_DECODE_INT64(dec, 1, &rc->x1);
  GLME_DECODE_INT64(dec, 2, &rc->y0);
  GLME_DECODE_INT64(dec, 3, &rc->y1);
  GLME_DECODE_END;
}


typedef struct data_t {
  double r;
  rect_t shape;
  uint64_t a;
} data_t;

int datacmp(data_t *a, data_t *b)
{
  int k;
  if (a->r != b->r)
    return a->r > b->r ? -1 : 1;
  if (a->a != b->a)
    return a->a > b->a ? -1 : 1;
  return rectcmp(&a->shape, &b->shape);
}

int glme_encode_data_t(glme_encoder_t *enc, data_t *msg)
{
  GLME_ENCODE_STDDEF;
  GLME_ENCODE_TYPE(enc, MSG_DATA_ID);
  GLME_ENCODE_DELTA(enc);
  GLME_ENCODE_DOUBLE(enc, 0, msg->r);
  GLME_ENCODE_STRUCT(enc, 1, &msg->shape, glme_encode_rect_t);
  GLME_ENCODE_UINT64(enc, 2, msg->a);
  GLME_ENCODE_END;
}


int glme_decode_data_t(glme_decoder_t *dec, data_t *msg)
{
  GLME_DECODE_STDDEF;
  GLME_DECODE_TYPE(dec, MSG_DATA_ID);
  GLME_DECODE_DELTA(dec);
  GLME_DECODE_DOUBLE(dec, 0, &msg->r);
  GLME_DECODE_STRUCT(dec, 1, &msg->shape, glme_decode_rect_t);
  GLME_DECODE_UINT64(dec, 2, &msg->a);
  GLME_DECODE_END;
}

void printdata(data_t *t)
{
  fprintf(stderr, "r    : %f\n", t->r);
  fprintf(stderr, "s.x0 : %ld\n", t->shape.x0);
  fprintf(stderr, "s.y0 : %ld\n", t->shape.y0);
  fprintf(stderr, "s.x1 : %ld\n", t->shape.x1);
  fprintf(stderr, "s.y1 : %ld\n", t->shape.y1);
  fprintf(stderr, "a    : %lu\n", t->a);
}

int main(int argc, char **argv)
{
  data_t msg, rcv;
  glme_encoder_t encoder;
  glme_decoder_t decoder;
  int k, n, doencode = 1;
  int pipefd[2];
  pid_t cpid;
  uint64_t rlen, nbytes;
  long vlen;
  double scale = 1.0;

  memset(&msg, 0, sizeof(msg));
  memset(&rcv, 0, sizeof(rcv));
  glme_encoder_init(&encoder, 1024);
  glme_decoder_init(&decoder, 1024);

  nbytes = sizeof(data_t);

  // generate random doubles
  srand48(time(0));
  msg.r = 0.0;
  msg.shape.x0 = 0;
  msg.shape.y0 = 0;
  msg.shape.x1 = 10;
  msg.shape.y1 = 10;
  msg.a = 0xFF;

  //dataprint(&msg);
  if (argc > 2) {
    glme_encode_data_t(&encoder, &msg);
    glme_encoder_writem(&encoder, 1);
    exit(0);
  }

  if (pipe(pipefd) == -1) {
    perror("pipe");
    exit(1);
  }
  
  cpid = fork();
  if (cpid == -1) {
    perror("fork");
    exit(1);
  }
  if (cpid == 0) {
    // encoder in child
    close(pipefd[0]);

    glme_encode_data_t(&encoder, &msg);
    glme_encoder_writem(&encoder, pipefd[1]);
    close(pipefd[1]);
    exit(0);
  } else {
    // decoder in master
    close(pipefd[1]);

    n = glme_decoder_readm(&decoder, pipefd[0]);
    // decode buffer starts with message length
    n = glme_decode_uint64(&decoder, &rlen);
    n = glme_decode_data_t(&decoder, &rcv);
    if (n < 0) {
      fprintf(stderr, "decode_error... %d\n", n);
    }
    close(pipefd[0]);
    wait(NULL);
  }
  n = datacmp(&msg, &rcv);
  fprintf(stderr, "decoded == encoded: %s [encoded length %ld/%ld bytes]\n",
	  n == 0 ? "YES" : "NO", rlen, nbytes);
  printdata(&rcv);
  exit(n);
}

