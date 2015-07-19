
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "glme.h"

#define NUMTESTS 20

#define MSG_LIST_ID 32
#define MSG_LINK_ID 33


typedef struct link {
  int val;
  struct link *next;
} link_t;

typedef struct list {
  struct link *head;
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


int encode_link(glme_buf_t *gb, const void *vptr)
{
  const struct link *lnk = (const struct link *)vptr;
  GLME_ENCODE_STDDEF(gb);
  GLME_ENCODE_STRUCT_START(gb);

  GLME_ENCODE_FLD_INT(gb, lnk->val, 0);
  GLME_ENCODE_FLD_STRUCT(gb, MSG_LINK_ID, lnk->next, encode_link);

  GLME_ENCODE_STRUCT_END(gb);
  GLME_ENCODE_RETURN(gb);
}

int decode_link(glme_buf_t *gb, void *ptr)
{
  struct link *lnk = (struct link *)ptr;
  GLME_DECODE_STDDEF(gb);
  GLME_DECODE_STRUCT_START(gb);

  GLME_DECODE_FLD_INT(gb, lnk->val, 0);
  GLME_DECODE_FLD_STRUCT_PTR(gb, MSG_LINK_ID, lnk->next, decode_link);

  GLME_DECODE_STRUCT_END(gb);
  GLME_DECODE_RETURN(gb);
}


int encode_list(glme_buf_t *gb, const void *ptr)
{
  const struct list *l = (const struct list *)ptr;
  GLME_ENCODE_STDDEF(gb);
  GLME_ENCODE_STRUCT_START(gb);

  GLME_ENCODE_FLD_STRUCT(gb, MSG_LINK_ID, l->head, encode_link);
  
  GLME_ENCODE_STRUCT_END(gb);
  GLME_ENCODE_RETURN(gb);
}

int decode_list(glme_buf_t *gb, void *ptr)
{
  struct list *l = (struct list *)ptr;
  GLME_DECODE_STDDEF(gb);
  GLME_DECODE_STRUCT_START(gb);

  GLME_DECODE_FLD_STRUCT_PTR(gb, 33, l->head, decode_link);

  GLME_DECODE_STRUCT_END(gb);
  GLME_DECODE_RETURN(gb);
}

static inline
int64_t read_tsc()
{
  unsigned reslo, reshi;

  // serialize (save ebx)
  __asm__ __volatile__  (
			 "xorl %%eax,%%eax \n cpuid \n"
			 ::: "%eax", "%ebx", "%ecx", "%edx");

  // read TSC, store edx:eax in res
  __asm__ __volatile__  (
			 "rdtsc\n"
			 : "=a" (reslo), "=d" (reshi) );

  // serialize again
  __asm__ __volatile__  (               
			 "xorl %%eax,%%eax \n cpuid \n"
			 ::: "%eax", "%ebx", "%ecx", "%edx");

  return ((uint64_t)reshi << 32) | reslo;
}

uint64_t run_test(uint64_t clocks[], glme_buf_t *encoder, list_t *msg)
{
  int i, j, k, n;
  uint64_t before, nb;

  for (k = 0; k < NUMTESTS; k++) {
    before = read_tsc();
    // ------ start of test ---

    n = glme_encode_struct(encoder, MSG_LIST_ID, msg, encode_list);

    // ------ end of test -----
    clocks[k] = read_tsc() - before;

    glme_buf_clear(encoder);
    if (k == 0)
      nb = (uint64_t)n;
  }

  return n == 0 ? 0 : nb;
}

double gbytes_per_sec(double cycles_per_byte, double clockrate)
{
  // clockrate = N Ghz (N cycles/sec)
  // cycles_per_bytes
  // ==> gbytes_per_sec = clockrate/cycles_per_byte
  return clockrate/cycles_per_byte;
}

int main(int argc, char **argv)
{
  int n, i, k, minind, maxind, encode, opt;
  uint64_t before, overhead, clocks[NUMTESTS];
  uint64_t tmin, tmax;
  double tavg, tcalc, bps_min, bps_avg, bps_max, clockrate;

  list_t lst;
  link_t *elems;
  
  glme_buf_t encoder;
  uint64_t rlen, nbytes, sbytes;

  long vlen;

  clockrate = 2.40;  // GHz
  // this many list entries --> max depth recursive calls 
  vlen = 20000;

  memset(&lst, 0, sizeof(lst));

  while ((opt = getopt(argc, argv, "R:")) != -1) {
    switch (opt) {
    case 'R':
      clockrate = strtod(optarg, (char **)0);
      break;
    default:
      printf("perf_da1 [-R clockrate] [arraylen]\n");
      exit(1);
    }
  }

  if (optind < argc)
    vlen = strtol(argv[optind], (char **)0, 10);

  glme_buf_init(&encoder, sizeof(link_t)*vlen);

  // generate random doubles
  nbytes = vlen*sizeof(link_t);
  elems = (link_t *)calloc(vlen, sizeof(link_t));
  for (k = 0; k < vlen-1; k++) {
    elems[k].val = k;
    elems[k].next = &elems[k+1];
  }
  elems[vlen-1].val = vlen-1;
  elems[vlen-1].next = (link_t *)0;
  lst.head = elems;

  
  sbytes = vlen*sizeof(link_t) + sizeof(list_t);

  // calculate overhead
  for (i = 0; i < NUMTESTS; i++) {
    before = read_tsc();
    clocks[i] = read_tsc() - before;
  }
  overhead = clocks[0];
  for (i = 0; i < NUMTESTS; i++) {
    if (clocks[i] < overhead)
      overhead = clocks[i];
  }

  // -------------------------------------------------------
  // run & measure
  nbytes = run_test(clocks, &encoder, &lst);

  // -------------------------------------------------------

  tmin = clocks[0];
  tmax = overhead;
  tavg = 0.0;
  minind = maxind = -1;
  for (i = 0; i < NUMTESTS; i++) {
    if (clocks[i] >= overhead) {
      clocks[i] -= overhead;
    } else {
      clocks[i] = 0;
    }
    if (clocks[i] < tmin) {
      tmin = clocks[i]; minind = i;
    }
    if (clocks[i] > tmax) {
      tmax = clocks[i]; maxind = i;
    }
  }
  n = 0;
  for (i = 0; i < NUMTESTS; i++) {
    tavg += ((double)clocks[i] - tavg) / (n+1);
    n++;
  }
  // bytes per sec (GB/s)
  bps_max = clockrate/((double)tmin/sbytes);
  bps_avg = clockrate/((double)tavg/sbytes);
  bps_min = clockrate/((double)tmax/sbytes);
  printf("[%7ld -> %7ld]: %.5f  %.5f  %.5f (GB/s)\n",
	 sbytes, nbytes, bps_min, bps_avg, bps_max);


}
