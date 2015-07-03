
// Copyright (c) Harri Rautila, 2015


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/queue.h>

#include <uv.h>
#include <glme/glme.h>

typedef struct data_t {
  int  blen;
  unsigned char *base;
} data_t;


#define MSG_DATA_ID 32

int glme_encode_data_t(glme_encoder_t *enc, data_t *msg, int typeid)
{
  GLME_ENCODE_STDDEF;
  GLME_ENCODE_TYPE(enc, typeid);
  GLME_ENCODE_DELTA(enc);
  GLME_ENCODE_INT(enc, 0, msg->blen);
  GLME_ENCODE_BYTES(enc, 1, msg->base, msg->blen);
  GLME_ENCODE_END;
}

typedef struct worker_s {
  // make this 'subclass' of uv_tcp_t struct
  uv_tcp_t stream;

  // static uv_bufs
  uv_buf_t outbuf[2];
  uv_write_t outreq;

  // prefix data and length
  char outtmp[12];
  char outlen;

  // application fields
  int start_len;
  int end_len;
  int current_len;
  int increment;

  int done;
} worker_t;


int make_data(glme_encoder_t *encoder, data_t *data, int nc)
{
  int k;
  data->blen = nc;
  data->base = malloc(data->blen);
  for (k = 0; k < data->blen; k++) {
    data->base[k] = k % 256;
  }
  return glme_encode_data_t(encoder, data, MSG_DATA_ID);
}

/*
 * CB for write requests.
 */
void on_write(uv_write_t *req, int status)
{
  worker_t *wrkr = (worker_t *)req->handle->data;
  glme_encoder_t enc;
  data_t data;
  int n;

  fprintf(stderr, "..on-write: %d [%d]\n", status, wrkr->current_len);
  // release old output buffer
  free(wrkr->outbuf[1].base);

  // next message size
  wrkr->current_len += wrkr->increment;
  if (wrkr->current_len >= wrkr->end_len) {
    wrkr->done = 1;
    return;
  }

  // initilize encoder
  glme_encoder_init(&enc, wrkr->current_len);
  // create and encode data; n is payload length
  n = make_data(&enc, &data, wrkr->current_len);

  // place it to the second uvbuf entry
  wrkr->outbuf[1] = uv_buf_init(glme_encoder_data(&enc), glme_encoder_bytes(&enc));
  // disown data buffer
  enc.owner = 0;
  // close encoder
  glme_encoder_close(&enc);

  // initialize for payload length encoding
  glme_encoder_make(&enc, wrkr->outtmp, sizeof(wrkr->outtmp));
  // encode length
  glme_encode_uint(&enc, n);
  // place to first uvbuf entry
  wrkr->outbuf[0] = uv_buf_init(glme_encoder_data(&enc), glme_encoder_bytes(&enc));
  glme_encoder_close(&enc);

  // write to network
  uv_write(&wrkr->outreq, (uv_stream_t *)&wrkr->stream, wrkr->outbuf, 2, on_write);
}


/**
 * CB for opened connection
 */
void on_connect(uv_connect_t *connect, int status)
{
  data_t data;
  glme_encoder_t enc;
  int n;

  if (status < 0) {
    fprintf(stderr, "Connection failer: %s\n", 
            uv_strerror(uv_last_error(connect->handle->loop)));
    return;
  }

  uv_stream_t *stream = (uv_stream_t *)connect->handle;
  // active handle
  //uv_read_start(stream, alloc_buffer, on_read);

  worker_t *wrkr = (worker_t *)stream->data;

  // write first message to stream

  glme_encoder_init(&enc, wrkr->start_len);
  // create and encode payload
  n = make_data(&enc, &data, wrkr->start_len);
  // place it to second uvbuf entry
  wrkr->outbuf[1] = uv_buf_init(glme_encoder_data(&enc), glme_encoder_bytes(&enc));
  // disown buffer
  enc.owner = 0;
  // reset encoder
  glme_encoder_close(&enc);

  // recreate for payload length encoding
  glme_encoder_make(&enc, wrkr->outtmp, sizeof(wrkr->outtmp));
  // encode payload length
  glme_encode_uint(&enc, n);
  // place to first uvbuf entry
  wrkr->outbuf[0] = uv_buf_init(glme_encoder_data(&enc), glme_encoder_bytes(&enc));
  glme_encoder_close(&enc);

  // write to network
  uv_write(&wrkr->outreq, (uv_stream_t *)&wrkr->stream, wrkr->outbuf, 2, on_write);

  free(connect);
}

int main(int argc, char **argv)
{
  worker_t wrkr;
  struct sockaddr_in addr;
  uv_loop_t *loop = uv_default_loop();

  if (argc >= 4) {
    wrkr.start_len = strtol(argv[1], (char **)0, 10);
    wrkr.end_len   = strtol(argv[2], (char **)0, 10);
    wrkr.increment = strtol(argv[3], (char **)0, 10);
  } else {
    wrkr.start_len = 10;
    wrkr.end_len = 20;
    wrkr.increment = 4;
  }
  wrkr.current_len = wrkr.start_len;
  wrkr.done = 0;

  uv_tcp_init(loop, &wrkr.stream);
  wrkr.stream.data = &wrkr;

  addr = uv_ip4_addr("127.0.0.1", 4434);

  uv_connect_t *conreq = (uv_connect_t *)malloc(sizeof(uv_connect_t));

  uv_tcp_connect(conreq, &wrkr.stream, addr, on_connect);

  while (wrkr.done == 0) {
    uv_run(loop, UV_RUN_NOWAIT);
  }
  return 0;
}

// Local Variables:
// indent-tabs-mode: nil
// End:
