
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "glme.h"

// Transmitting structure from process to process 

#define MMAX (1 << 24)

typedef struct data_t {
  double vec[16];
} data_t;

int datacmp(data_t *a, data_t *b)
{
  int k;
  for (k = 0; k < 16; k++) {
    if (a->vec[k] != b->vec[k])
      return a->vec[k] > b->vec[k] ? -1 : 1;
  }
  return 0;
}


void dataprint(data_t *a, data_t *b)
{
  int k;
  for (k = 0; k < 16; k++) {
    fprintf(stderr, "%2d: %.4f  %.4f\n", k, a->vec[k], b->vec[k]);
  }
}

#define MSG_DATA_ID 32

int encode_data_t(glme_buf_t *enc, const void *ptr)
{
  const data_t *msg = (const data_t *)ptr;
  GLME_ENCODE_STDDEF(enc);
  GLME_ENCODE_STRUCT_START(enc);
  GLME_ENCODE_FLD_FLOAT_VECTOR(enc, msg->vec, glme_encode_value_double);
  GLME_ENCODE_STRUCT_END(enc);
  GLME_ENCODE_RETURN(enc);
}


int decode_data_t(glme_buf_t *dec, void *ptr)
{
  data_t *msg = (data_t *)ptr;
  GLME_DECODE_STDDEF(dec);
  GLME_DECODE_STRUCT_START(dec);
  // must use _VECTOR version as vec is not a pointer
  GLME_DECODE_FLD_FLOAT_VECTOR(dec, msg->vec, glme_decode_value_double);
  GLME_DECODE_STRUCT_END(dec);
  GLME_DECODE_RETURN(dec);
}


int main(int argc, char **argv)
{
  data_t msg, rcv;
  glme_buf_t encoder;
  glme_buf_t decoder;
  int k, n, doencode = 1;
  int pipefd[2];
  pid_t cpid;
  uint64_t rlen;
  double scale = 1.0;

  memset(&msg, 0, sizeof(msg));
  memset(&rcv, 0, sizeof(rcv));
  glme_buf_init(&encoder, 1024);
  glme_buf_init(&decoder, 1024);

  if (argc > 1) {
    scale = strtod(argv[1], (char **)0);
  }
  // generate random doubles
  srand48(time(0));
  for (k = 0; k < 16; k++) {
    msg.vec[k] = drand48()*scale;
  }

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
      printf("decode_error... %d\n", n);
    }
    close(pipefd[0]);
    kill(cpid);
    wait(NULL);
  }
  n = datacmp(&msg, &rcv);
  printf("decoded == encoded: %s\n", n == 0 ? "YES" : "NO");
  dataprint(&msg, &rcv);
  exit(n);
}
