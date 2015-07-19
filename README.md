
## GLME - A Binary Gob Like Message Encoding

GLME encoded stream is an adapted GOB stream where each data element is preceded by its type,
expressed in terms of small set of predefined types. Pointers are not encoded, but the things
they point to are. Compound types (structures) are identified by user selected unique positive
numbers. See file ENCODING for more information and examples in test directory.

Library supports encoding all basic types, strings, byte arrays, arrays of basic types,
structure, arrays of structures, acyclic data structures (like linked list) and to some
extend also cyclic data structures (like double linked list).

## Using

### Defining the message

Define the message as normal C structure. Select unique type id for the message.

```c
   // define the message fields
   typedef struct msg
   {
     unsigned int ui;
     int  i;
     double r;
     char vec[16];
     char *ptr;
   } msg_t;

   // typeid for this message
   #define MSG_ID 11
```

### Define encoder and decoder

Define encoder and decoder functions. Encoder function returns number bytes writen to the
specified encoder object. On error negative error code is returned.

The decoder function returns number of bytes read from the specified decoder object. Negative
error code indicates error.

```c
   #include <glme.h>

   // message encoding function
   int msg_encoder(glme_buf_t *enc, const void *p)
   {
     const msg_t *m = (const msg_t *)p;
     GLME_ENCODE_STDDEF(enc);
     GLME_ENCODE_STRUCT_START(enc);
     GLME_ENCODE_FLD_UINT(enc, m->ui, 0);
     GLME_ENCODE_FLD_INT(enc, m->i, 0);
     GLME_ENCODE_FLD_DOUBLE(enc, m->r, 0.0);
     GLME_ENCODE_FLD_VECTOR(enc, m->vec, sizeof(m->vec));
     GLME_ENCODE_FLD_STRING(enc, m->ptr);
     GLME_ENCODE_STRUCT_END(enc);
     GLME_ENCODE_RETURN(enc);     
   }

   int msg_decoder(glme_buf_t *dec, void *p)
   {
     msg_t *m = (msg_t *)p;
     GLME_DECODE_STDDEF)dec);
     GLME_DECODE_STRUCT_START(dec);
     GLME_DECODE_FLD_UINT(dec, m->ui, 0);
     GLME_DECODE_FLD_INT(dec, m->i, 0);
     GLME_DECODE_FLD_DOUBLE(dec, &m->r, 0.0);
     GLME_DECODE_FLD_VECTOR(dec, m->vec, sizeof(m->vec));
     GLME_DECODE_FLD_STRING(dec, &m->ptr);
     GLME_DECODE_STRUCT_END(dec);
     GLME_DECODE_RETURN(dec);
   }

```

### Sending the message

Initialize encoder, construct data to send, encode it and write out to open file descriptor.

```c
   #include <glme.h>
   int main(int argc, char **argv)
   {
      msg_t msg;
      glme_buf_t encoder;

      glme_buf_init(&encoder, 1024);

      msg.ui = 0xFF; msg.i = -777; msg.r = 17.0;
      strncpy(msg.vec, "hello", sizeof(msg.vec));
      msg.ptr = "world";
      
      glme_encode_struct(&encoder, MSG_ID, &msg, msg_encoder);
      // write message to stdout
      glme_buf_writem(&encoder, 1);
   }
```

### Reading the message

Initialize decoder, read the data, decode and use.

```c
   #include <glme.h>
   #define MMAX (1<<24)

   int main(int argc, char **argv)
   {
      msg_t msg;
      glme_buf_t decoder;

      glme_buf_init(&decoder, 1024);
      
      // read the message; 
      glme_buf_readm(&decoder, 0, MMAX);
      // decode the message
      glme_decode_struct(&decoder, MSG_ID, &msg, msg_decoder);
   }
```

### Defining the encoder and decoder for dynamic structure

```c
    // simple list
    typedef struct link {
       int value;
       struct link *next;
    } link_t;

    typedef struct list {
       link_t *head;
    } list_t;
    
    #define LINK_ID 10
    #define LIST_ID 11
```

Encoder functions for both types

```c
   int encode_link_t(glme_buf_t *enc, const void *p)
   {
     const link_t *ln = (const link_t *)p;
     GLME_ENCODE_STDDEF(enc);
     GLME_ENCODE_STRUCT_START(enc);
     GLME_ENCODE_FLD_INT(enc, ln->value, 0);
     GLME_ENCODE_FLD_STRUCT(enc, LINK_ID, ln->next, encode_link_t);
     GLME_ENCODE_STRUCT_END(enc);
     GLME_ENCODE_RETURN(enc);
   }

   int encode_list_t(glme_buf_t *enc, const void *p)
   {
     const link_t *lst = (const link_t *)p;
     GLME_ENCODE_STDDEF(enc);
     GLME_ENCODE_STRUCT_START(enc);
     GLME_ENCODE_FLD_STRUCT(enc, LINK_ID, lst->head, encode_link_t);
     GLME_ENCODE_STRUCT_END(enc);
     GLME_ENCODE_RETURN(enc);
   }
```


Decoder functions for both types

```c
   int decode_link_t(glme_buf_t *dec, link_t *ln)
   {
     link_t *ln = (link_t *)p;
     GLME_DECODE_STDDEF(dec);
     GLME_DECODE_STRUCT_START(dec);
     GLME_DECODE_FLD_INT(dec, ln->value, 0);
     GLME_DECODE_FLD_STRUCT_PTR(dec, LINK_ID, ln->next, decode_link_t);
     GLME_DECODE_END(dec);
     GLME_ENCODE_RETURN(enc);
   }

   int decode_list_t(glme_buf_t *dec, list_t *lst)
   {
     link_t *lst = (link_t *)p;
     GLME_DECODE_STDDEF(dec);
     GLME_DECODE_STRUCT_START(dec);
     GLME_DECODE_FLD_STRUCT_PTR(dec, LINK_ID, ln->next, decode_link_t);
     GLME_DECODE_END(dec);
     GLME_ENCODE_RETURN(enc);
   }
```

