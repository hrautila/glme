
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "glme.h"

#define NUMTESTS 20

typedef struct data_t {
  double *vec;
  uint64_t vlen;
} data_t;

int datacmp(data_t *a, data_t *b)
{
  int k;
  if (a->vlen != b->vlen)
    return a->vlen > b->vlen ? -1 : 1;

  for (k = 0; k < a->vlen; k++) {
    if (a->vec[k] != b->vec[k])
      return a->vec[k] > b->vec[k] ? -1 : 1;
  }
  return 0;
}

#define MSG_DATA_ID 32

int glme_encode_data_t(glme_buf_t *enc, data_t *msg)
{
  GLME_ENCODE_STDDEF;
  GLME_ENCODE_TYPE(enc, MSG_DATA_ID);
  GLME_ENCODE_DELTA(enc);
  GLME_ENCODE_ARRAY(enc, 0, msg->vec, msg->vlen, double);
  GLME_ENCODE_END;
}


int glme_decode_data_t(glme_buf_t *dec, data_t *msg)
{
  GLME_DECODE_STDDEF;
  GLME_DECODE_TYPE(dec, MSG_DATA_ID);
  GLME_DECODE_DELTA(dec);
  GLME_DECODE_VAR_ARRAY(dec, 0, msg->vec, msg->vlen, double);
  GLME_DECODE_END;
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

uint64_t run_test(uint64_t clocks[], glme_buf_t *encoder, data_t *msg)
{
  int i, j, k, n;
  uint64_t before, nb;

  for (k = 0; k < NUMTESTS; k++) {
    before = read_tsc();
    // ------ start of test ---

    n = glme_encode_data_t(encoder, msg);

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
  double tavg, tcalc, bps_min, bps_avg, bps_max, nps_max, clockrate;

  data_t msg, rcv;
  glme_buf_t encoder;
  uint64_t rlen, nbytes, sbytes;

  long vlen;
  double scale = 1.0;

  clockrate = 2.40;  // GHz
  vlen = 100000;

  memset(&msg, 0, sizeof(msg));
  memset(&rcv, 0, sizeof(rcv));

  vlen = 100000;

  while ((opt = getopt(argc, argv, "R:S:")) != -1) {
    switch (opt) {
    case 'R':
      clockrate = strtod(optarg, (char **)0);
      break;
    case 'S':
      scale = strtod(optarg, (char **)0);
      break;
    default:
      printf("perf_da1 [-R clockrate -S scale] [arraylen]\n");
      exit(1);
    }
  }

  if (optind < argc)
    vlen = strtol(argv[optind], (char **)0, 10);

  glme_buf_init(&encoder, 9*vlen);

  // generate random doubles
  srand48(time(0));
  nbytes = vlen*sizeof(double);
  msg.vec = malloc(vlen*sizeof(double));
  msg.vlen = vlen;
  for (k = 0; k < vlen; k++) {
    msg.vec[k] = drand48()*scale;
  }
  sbytes = vlen*sizeof(double) + sizeof(msg);

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
  nbytes = run_test(clocks, &encoder, &msg);

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
  nps_max = clockrate/((double)tmin/nbytes);
  printf("[%7ld -> %7ld]: %.5f  %.5f  %.5f [%.5f] (GB/s)\n",
	 sbytes, nbytes, bps_min, bps_avg, bps_max, nps_max);


}
