
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "glme.h"

// Linked list from process to process

#define MSG_LIST_ID 32
#define MSG_LINK_ID 33

#define MMAX (1 << 24)

typedef struct link {
  int val;
  struct link *next;
} link_t;


int encode_link_t(glme_buf_t *enc, const void *ptr)
{
  const link_t *rc = (const link_t *)ptr;
  GLME_ENCODE_STDDEF(enc);
  GLME_ENCODE_STRUCT_START(enc);
  GLME_ENCODE_FLD_INT(enc, rc->val, 0);
  GLME_ENCODE_FLD_STRUCT(enc, MSG_LINK_ID, rc->next, 0);
  GLME_ENCODE_STRUCT_END(enc);
  GLME_ENCODE_RETURN(enc);
}

int decode_link_t(glme_buf_t *dec, void *ptr)
{
  link_t *rc = (link_t *)ptr;
  GLME_DECODE_STDDEF(dec);
  GLME_DECODE_STRUCT_START(dec);
  GLME_DECODE_FLD_INT(dec, rc->val, 0);
  GLME_DECODE_FLD_STRUCT_PTR(dec, MSG_LINK_ID, rc->next, 0);
  GLME_DECODE_STRUCT_END(dec);
  GLME_DECODE_RETURN(dec);
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

int encode_list_t(glme_buf_t *enc, const void *ptr)
{
  const list_t *lst = (const list_t *)ptr;
  GLME_ENCODE_STDDEF(enc);
  GLME_ENCODE_STRUCT_START(enc);
  GLME_ENCODE_FLD_STRUCT(enc, MSG_LINK_ID, lst->head, 0);
  GLME_ENCODE_STRUCT_END(enc);
  GLME_ENCODE_RETURN(enc);
}


int decode_list_t(glme_buf_t *dec, void *ptr)
{
  list_t *lst = (list_t *)ptr;
  GLME_DECODE_STDDEF(dec);
  GLME_DECODE_STRUCT_START(dec);
  GLME_DECODE_FLD_STRUCT_PTR(dec, MSG_LINK_ID, lst->head, 0);
  GLME_DECODE_STRUCT_END(dec);
  GLME_DECODE_RETURN(dec);
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
  glme_buf_t encoder;
  glme_buf_t decoder;
  glme_base_t base;
  int k, n, doencode = 1;
  int pipefd[2];
  pid_t cpid;
  uint64_t rlen, nbytes;
  list_t snd, rcv, *rptr;
  link_t s0, s1, s2;
  link_t *links;
  long vlen;
  size_t ncount = 3;

  glme_spec_t specs[] = {
    (glme_spec_t){ MSG_LINK_ID, sizeof(link_t), encode_link_t, decode_link_t },
    (glme_spec_t){ MSG_LIST_ID, sizeof(list_t), encode_list_t, decode_list_t }
  };

  memset(&snd, 0, sizeof(snd));
  memset(&rcv, 0, sizeof(rcv));
  glme_buf_init(&encoder, 10240);
  glme_buf_init(&decoder, 10240);

  glme_base_init(&base, specs, 2, (glme_allocator_t *)0);
  decoder.base = &base;
  encoder.base = &base;

  if (argc > 1)
    ncount = strtoul(argv[1], (char **)0, 10);

  // create list;
  links = (link_t *)malloc(ncount * sizeof(link_t));
  for (k = 0; k < ncount-1; k++) {
    links[k].val = k+1;
    links[k].next = &links[k+1];
  }
  links[ncount-1] = (link_t){ncount, (link_t *)0};
  snd = (list_t){&links[0]};

#if 0
  s2 = (link_t){2, (link_t *)0};
  s1 = (link_t){1, &s2};
  s0 = (link_t){0, &s1};
  snd = (list_t){&s0};
#endif
  nbytes = sizeof(list_t) + ncount*sizeof(link_t);

  //dataprint(&msg);
  if (argc > 2) {
    glme_encode_struct(&encoder, MSG_LIST_ID, &snd, encode_list_t);
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

    if ((n = glme_encode_struct(&encoder, MSG_LIST_ID, &snd, encode_list_t)) < 0)
      fprintf(stderr, "..encode error... %d\n", n);
    glme_buf_writem(&encoder, pipefd[1]);
    close(pipefd[1]);
    exit(0);
  } else {
    // decoder in master
    close(pipefd[1]);

    n = glme_buf_readm(&decoder, pipefd[0], MMAX);
    rptr = &rcv;
    n = glme_decode_struct(&decoder, MSG_LIST_ID, (void **)&rptr, 0, decode_list_t);
    if (n < 0) {
      fprintf(stderr, "decode_error... %d\n", n);
    }
    close(pipefd[0]);
    wait(NULL);
  }
  n = listcmp(&snd, &rcv);
  fprintf(stderr, "decoded == encoded: %s [encoded length %ld/%ld bytes]\n",
	  n == 0 ? "YES" : "NO", glme_buf_len(&decoder), nbytes);
  if (ncount < 10)
    printlist(&rcv);
  exit(n);
}

