
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

int glme_decode_data_t(glme_decoder_t *dec, data_t *msg)
{
  GLME_DECODE_STDDEF;
  GLME_DECODE_TYPE(dec, MSG_DATA_ID);
  GLME_DECODE_DELTA(dec);
  GLME_DECODE_INT(dec, 0, &msg->blen);
  GLME_DECODE_BYTES(dec, 1, (void **)&msg->base);
  GLME_DECODE_END;
}

typedef struct client_s {
  // this 'subclass' of uv_tcp_t
  uv_tcp_t source;

  // io-state; 0 = reading prefix; 1 = reading body
  int state;

  // number of bytes read
  int nread;

  // field for reading message length prefix
  char intmp[16];
  int inalloc;

  // fields for reading message data
  char *inbuf;
  unsigned int inlen;

  int done;
} client_t;

static inline
void client_init(client_t *clnt)
{
  clnt->state = 0;
  clnt->nread = 0;
  clnt->inalloc = 0;
  clnt->inlen = 0;
  clnt->done = 0;
}


/**
 * CB for allocating buffer for reads.
 */
uv_buf_t alloc_buffer(uv_handle_t *handle, size_t ssize)
{
  client_t *clnt = (client_t *)handle->data;

  if (clnt->state == 0) {
    // reading message length
    if (clnt->inalloc == 0) {
      clnt->inalloc = 3;
      return uv_buf_init(clnt->intmp, clnt->inalloc);
    }
    clnt->inalloc = sizeof(clnt->intmp);
    return uv_buf_init(&clnt->intmp[3], clnt->inalloc-3);
  }

  // reading message body; allow data upto missing number of bytes
  return uv_buf_init(&clnt->inbuf[clnt->nread], clnt->inlen-clnt->nread);
}

/**
 * CB for reading data from network stream
 */
void on_read(uv_stream_t *stream, ssize_t nread, uv_buf_t buf)
{
  int n, ok;
  glme_decoder_t dec;
  data_t data;
  client_t *clnt = (client_t *)stream->data;
  
  if (nread < 0) {
    if (nread == -1) {
      fprintf(stderr, "Connection closed ...\n");
      clnt->done = 1;
      uv_close((uv_handle_t *)stream, NULL);
    }
    return;
  }

  if (nread == 0) {
    // buffer requested earlier was not needed. 
    if (clnt->state == 0) {
      // undo previous allocation request
      clnt->inalloc = clnt->inalloc == 3 ? 0 : 3;
    }
    return;
  }

  if (clnt->state == 0) {
    // reading message length prefix
    clnt->nread += nread;
    glme_decoder_make(&dec, clnt->intmp, sizeof(clnt->intmp), clnt->nread);
    if ((n = glme_decode_uint(&dec, &clnt->inlen)) < 0) {
      // under flow
      clnt->inlen = 0;
      return;
    }

    clnt->state = 1;
    clnt->inbuf = malloc(clnt->inlen);
    // if part of message in tmpbuf, copy it to the start inbuf
    if (n < clnt->nread) {
      memcpy(clnt->inbuf, &clnt->intmp[n], clnt->nread-n);
      clnt->nread -= n;
    } else {
      clnt->nread = 0;
    }
    return;
  }

  // here state == 1; we are reading the message body
  clnt->nread += nread;
  if (clnt->nread == clnt->inlen) {
    // got all of it

    glme_decoder_make(&dec, clnt->inbuf, clnt->inlen, clnt->inlen);
    n = glme_decode_data_t(&dec, &data);
    if (n > 0) {
      fprintf(stderr, ".... decoded %d bytes\n", n);
      free(data.base);
    } else {
      fprintf(stderr, ".... [%3d] decode error\n", n);
    }

    clnt->nread = 0;
    clnt->state = 0;
    clnt->inalloc = 0;
    clnt->inlen = 0;
    free(clnt->inbuf);
    clnt->inbuf = (unsigned char *)0;
  }
}

/**
 * CB for new connection requests.
 */
void on_connection(uv_stream_t *server, int status)
{
  if (status < 0) {
    fprintf(stderr, "New connection error: %s\n", 
            uv_strerror(uv_last_error(server->loop)));
    return;
  }
  
  client_t *clnt = (client_t *)malloc(sizeof(client_t));
  client_init(clnt);
  uv_tcp_init(server->loop, &clnt->source);

  if (uv_accept(server, (uv_stream_t *)&clnt->source) != 0) {
    fprintf(stderr, "Accept failed: %s\n", 
            uv_strerror(uv_last_error(server->loop)));
    uv_close((uv_handle_t *)&clnt->source, NULL);
    free(clnt);
    return;
  }

  // succesfull accept here
  clnt->source.data = (void *)clnt;
  uv_read_start((uv_stream_t *)&clnt->source, alloc_buffer, on_read);

  // unref server to force program termination when client disconnects
  uv_unref((uv_handle_t *)server);
}


int main(int argc, char **argv)
{
  uv_loop_t *loop = uv_default_loop();
  uv_tcp_t server;
  int e;
  struct sockaddr_in addr;

  uv_tcp_init(loop, &server);
  addr = uv_ip4_addr("0.0.0.0", 4434);

  uv_tcp_bind(&server, addr);
  e = uv_listen((uv_stream_t *)&server, 5, on_connection);
  if (e < 0) {
    fprintf(stderr, "Listen error: %s\n", 
            uv_strerror(uv_last_error(loop)));
    return 1;
  }

  uv_run(loop, UV_RUN_DEFAULT);
  return 0;
}

// Local Variables:
// indent-tabs-mode: nil
// End:
