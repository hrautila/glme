
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "glme/glme.h"

#define MSG_LIST_ID 32
#define MSG_LINK_ID 33


typedef struct link {
  int val;
  struct link *next;
} link_t;


int glme_encode_link_t(glme_encoder_t *enc, link_t *rc)
{
  GLME_ENCODE_STDDEF;
  GLME_ENCODE_TYPE(enc, MSG_LINK_ID);
  GLME_ENCODE_DELTA(enc);
  GLME_ENCODE_INT(enc, 0, rc->val);
  GLME_ENCODE_STRUCT_PTR(enc, 1, rc->next, glme_encode_link_t);
  GLME_ENCODE_END;
}

int glme_decode_link_t(glme_decoder_t *dec, link_t *rc)
{
  GLME_DECODE_STDDEF;
  GLME_DECODE_TYPE(dec, MSG_LINK_ID);
  GLME_DECODE_DELTA(dec);
  GLME_DECODE_INT(dec, 0, &rc->val);
  GLME_DECODE_STRUCT_PTR(dec, 1, rc->next, glme_decode_link_t, link_t);
  GLME_DECODE_END;
}


typedef struct list_t {
  link_t *head;
} list_t;

int listcmp(list_t *a, list_t *b)
{
  link_t *n0, *n1;
  for (n0 = a->head, n1 = b->head; n0 && n1; n0 = n0->next, n1 = n1->next) {
    if (n0->val != n1->val)
      return n0->val > n1->val ? -1 : 1;
  }
  if (n0)
    return -1;
  if (n1)
    return 1;
  return 0;
}

int glme_encode_list_t(glme_encoder_t *enc, list_t *lst)
{
  GLME_ENCODE_STDDEF;
  GLME_ENCODE_TYPE(enc, MSG_LIST_ID);
  GLME_ENCODE_DELTA(enc);
  GLME_ENCODE_STRUCT_PTR(enc, 0, lst->head, glme_encode_link_t);
  GLME_ENCODE_END;
}


int glme_decode_list_t(glme_decoder_t *dec, list_t *lst)
{
  GLME_DECODE_STDDEF;
  GLME_DECODE_TYPE(dec, MSG_LIST_ID);
  GLME_DECODE_DELTA(dec);
  GLME_DECODE_STRUCT_PTR(dec, 0, lst->head, glme_decode_link_t, link_t);
  GLME_DECODE_END;
}

void printlist(list_t *t)
{
  int k = 0;
  link_t *n;
  for (k = 0, n = t->head; n; n = n->next, k++) {
    fprintf(stderr, "[%d] val  : %d\n", k, n->val);
  }
}

int main(int argc, char **argv)
{
  glme_encoder_t encoder;
  glme_decoder_t decoder;
  int k, n, doencode = 1;
  int pipefd[2];
  pid_t cpid;
  uint64_t rlen, nbytes;
  list_t snd, rcv;
  link_t s0, s1, s2;
  long vlen;

  memset(&snd, 0, sizeof(snd));
  memset(&rcv, 0, sizeof(rcv));
  glme_encoder_init(&encoder, 1024);
  glme_decoder_init(&decoder, 1024);


  // create list;
  s2 = (link_t){2, (link_t *)0};
  s1 = (link_t){1, &s2};
  s0 = (link_t){0, &s1};
  snd = (list_t){&s0};

  nbytes = sizeof(list_t) + 3*sizeof(link_t);

  //dataprint(&msg);
  if (argc > 2) {
    glme_encode_list_t(&encoder, &snd);
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

    glme_encode_list_t(&encoder, &snd);
    glme_encoder_writem(&encoder, pipefd[1]);
    close(pipefd[1]);
    exit(0);
  } else {
    // decoder in master
    close(pipefd[1]);

    n = glme_decoder_readm(&decoder, pipefd[0]);
    // decode buffer starts with message length
    n = glme_decode_uint64(&decoder, &rlen);
    n = glme_decode_list_t(&decoder, &rcv);
    if (n < 0) {
      fprintf(stderr, "decode_error... %d\n", n);
    }
    close(pipefd[0]);
    wait(NULL);
  }
  n = listcmp(&snd, &rcv);
  fprintf(stderr, "decoded == encoded: %s [encoded length %ld/%ld bytes]\n",
	  n == 0 ? "YES" : "NO", rlen, nbytes);
  printlist(&rcv);
  exit(n);
}

