
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "glme.h"

// Structure with embedded structures from process to process

#define MSG_DATA_ID 32
#define MSG_RECT_ID 33

#define MMAX (1 << 24)

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

int encode_rect_t(glme_buf_t *enc, const void *ptr)
{
  const rect_t *rc = (const rect_t *)ptr;
  GLME_ENCODE_STDDEF(enc);
  GLME_ENCODE_STRUCT_START(enc);
  GLME_ENCODE_FLD_INT(enc, rc->x0, 0);
  GLME_ENCODE_FLD_INT(enc, rc->x1, 0);
  GLME_ENCODE_FLD_INT(enc, rc->y0, 0);
  GLME_ENCODE_FLD_INT(enc, rc->y1, 0);
  GLME_ENCODE_STRUCT_END(enc);
  GLME_ENCODE_RETURN(enc);
}

int decode_rect_t(glme_buf_t *dec, void *ptr)
{
  rect_t *rc = (rect_t *)ptr;
  GLME_DECODE_STDDEF(dec);
  GLME_DECODE_STRUCT_START(dec);
  GLME_DECODE_FLD_INT(dec, rc->x0, 0);
  GLME_DECODE_FLD_INT(dec, rc->x1, 0);
  GLME_DECODE_FLD_INT(dec, rc->y0, 0);
  GLME_DECODE_FLD_INT(dec, rc->y1, 0);
  GLME_DECODE_STRUCT_END(dec);
  GLME_DECODE_RETURN(dec);
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

int encode_data_t(glme_buf_t *enc, const void *ptr) 
{
  const data_t *msg = (const data_t *)ptr;
  GLME_ENCODE_STDDEF(enc);
  GLME_ENCODE_STRUCT_START(enc);
  GLME_ENCODE_FLD_DOUBLE(enc, msg->r, 0.0);
  GLME_ENCODE_FLD_STRUCT(enc, MSG_RECT_ID, &msg->shape, encode_rect_t);
  GLME_ENCODE_FLD_UINT(enc, msg->a, 0);
  GLME_ENCODE_STRUCT_END(enc);
  GLME_ENCODE_RETURN(enc);
}


int decode_data_t(glme_buf_t *dec, void *ptr)
{
  data_t *msg = (data_t *)ptr;
  GLME_DECODE_STDDEF(dec);
  GLME_DECODE_STRUCT_START(dec);
  GLME_DECODE_FLD_DOUBLE(dec, msg->r, 0.0);
  GLME_DECODE_FLD_STRUCT(dec, MSG_RECT_ID, msg->shape, decode_rect_t);
  GLME_DECODE_FLD_UINT(dec, msg->a, 0);
  GLME_DECODE_STRUCT_END(dec);
  GLME_DECODE_RETURN(dec);
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
  glme_buf_t encoder;
  glme_buf_t decoder;
  int k, n, doencode = 1;
  int pipefd[2];
  pid_t cpid;
  uint64_t rlen, nbytes;
  long vlen;
  double scale = 1.0;

  memset(&msg, 0, sizeof(msg));
  memset(&rcv, 0, sizeof(rcv));
  glme_buf_init(&encoder, 1024);
  glme_buf_init(&decoder, 1024);

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
    glme_encode_struct(&encoder, MSG_DATA_ID, &msg, encode_data_t);
    glme_buf_writem(&encoder, 1);
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

    glme_encode_struct(&encoder, MSG_DATA_ID, &msg, encode_data_t);
    glme_buf_writem(&encoder, pipefd[1]);
    close(pipefd[1]);
    exit(0);
  } else {
    // decoder in master
    close(pipefd[1]);

    n = glme_buf_readm(&decoder, pipefd[0], MMAX);
    n = glme_decode_struct(&decoder, MSG_DATA_ID, &rcv, decode_data_t);
    if (n < 0) {
      fprintf(stderr, "decode_error... %d\n", n);
    }
    close(pipefd[0]);
    wait(NULL);
  }
  n = datacmp(&msg, &rcv);
  fprintf(stderr, "decoded == encoded: %s [encoded length %ld/%ld bytes]\n",
	  n == 0 ? "YES" : "NO", glme_buf_len(&decoder), nbytes);
  printdata(&rcv);
  exit(n);
}

