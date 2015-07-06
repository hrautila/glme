
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "glme.h"

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

#define MSG_DATA_ID 32

int glme_encode_data_t(glme_buf_t *enc, data_t *msg, int typeid)
{
  GLME_ENCODE_STDDEF;
  GLME_ENCODE_TYPE(enc, typeid);
  GLME_ENCODE_DELTA(enc);
  GLME_ENCODE_ARRAY(enc, 0, msg->vec, 16, double);
  GLME_ENCODE_END;
}


int glme_decode_data_t(glme_buf_t *dec, data_t *msg, int typeid)
{
  GLME_DECODE_STDDEF;
  GLME_DECODE_TYPE(dec, typeid);
  GLME_DECODE_DELTA(dec);
  GLME_DECODE_ARRAY(dec, 0, msg->vec, 16, double);
  GLME_DECODE_END;
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
    glme_encode_data_t(&encoder, &msg, MSG_DATA_ID);
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

    glme_encode_data_t(&encoder, &msg, MSG_DATA_ID);
    glme_buf_writem(&encoder, pipefd[1]);
    close(pipefd[1]);
    exit(0);
  } else {
    // decoder in master
    close(pipefd[1]);

    n = glme_buf_readm(&decoder, pipefd[0], MMAX);
    n = glme_decode_data_t(&decoder, &rcv, MSG_DATA_ID);
    if (n < 0) {
      printf("decode_error... %d\n", n);
    }
    close(pipefd[0]);
    wait(NULL);
  }
  n = datacmp(&msg, &rcv);
  printf("decoded == encoded: %s\n", n == 0 ? "YES" : "NO");
  exit(n);
}
