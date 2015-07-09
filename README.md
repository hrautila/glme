
## GLME - Gob Like Message Encoding

GLME is library for descriping messages as C-structures, encoding them to binary format
and decoding the transmitted values from the binary byte stream. The binary format is
based on the GOB encoding and its predefined types. Encoded messages are not
self-descriping. Messages are sent as a pair (LENGTH, STRUCT). LENGTH is the length,
in bytes, of the encoded struct stream. The STRUCT is a pair (ID, FIELDS) where ID
is a signed integer uniquely identifying the struct type. The FIELDS element is a
sequence of (delta, value) pairs. Each value is sent using the standard encoding for
its type, If a field has the zero value for its type, it is omitted from the transmission.
Delta is the index offset from the previous structure field to this field. If delta is
greater than one fields have been omitted from the encoded stream. End of the type is
indicated with zero delta value.

Messages are descripted as standard C structures. Message or structure type spesific
encoding and decoding functions are written with GLME helper macros. Fields may be
basic C-types - signed or unsigned ints, single or double precision floating point
numbers, fixed size vectors, variable size strings, arrays of base types, embedded
structs, pointers to structs or non-cyclic data structures (like linked list,
binary trees). 

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
   int msg_encoder(glme_buf_t *enc, msg_t *m)
   {
     GLME_ENCODE_STDDEF;
     GLME_ENCODE_TYPE(enc, MSG_ID);
     GLME_ENCODE_DELTA(enc);
     GLME_ENCODE_UINT(enc, 0, m->ui);
     GLME_ENCODE_INT(enc, 1, m->i);
     GLME_ENCODE_DOUBLE(enc, 2, m->r);
     GLME_ENCODE_VEC(enc, 3, m->vec, sizeof(m->vec));
     GLME_ENCODE_STR(enc, 4, m->ptr);
     GLME_ENCODE_END(enc);
   }

   int msg_decoder(glme_buf_t *dec, msg_t *m)
   {
     GLME_DECODE_STDDEF;
     GLME_DECODE_TYPE(dec, MSG_ID);
     GLME_DECODE_DELTA(dec);
     GLME_DECODE_UINT(dec, 0, &m->ui);
     GLME_DECODE_INT(dec, 1, &m->i);
     GLME_DECODE_DOUBLE(dec, 2, &m->r);
     GLME_DECODE_VEC(dec, 3, m->vec, sizeof(m->vec));
     GLME_DECODE_STR(dec, 4, &m->ptr);
     GLME_DECODE_END(dec);
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
      
      msg_encode(&encoder, &msg);
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
      msg_decode(&decoder, &msg);
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
   int encode_link_t(glme_buf_t *enc, link_t *ln)
   {
     GLME_ENCODE_STDDEF;
     GLME_ENCODE_TYPE(enc, LINK_ID);
     GLME_ENCODE_DELTA(enc);
     GLME_ENCODE_INT(enc, 0, ln->value);
     GLME_ENCODE_STRUCT_PTR(enc, 1, ln->next, encode_link_t);
     GLME_ENCODE_END(enc);
   }

   int encode_list_t(glme_buf_t *enc, list_t *lst)
   {
     GLME_ENCODE_STDDEF;
     GLME_ENCODE_TYPE(enc, LIST_ID);
     GLME_ENCODE_DELTA(enc);
     GLME_ENCODE_STRUCT_PTR(enc, 0, lst->head, encode_link_t);
     GLME_ENCODE_END(enc);
   }
```


Decoder functions for both types

```c
   int decode_link_t(glme_buf_t *dec, link_t *ln)
   {
     GLME_DECODE_STDDEF;
     GLME_DECODE_TYPE(dec, LINK_ID);
     GLME_DECODE_DELTA(dec);
     GLME_DECODE_INT(dec, 0, &ln->value);
     GLME_DECODE_STRUCT_PTR(dec, 1, ln->next, decode_link_t, link_t);
     GLME_DECODE_END(dec);
   }

   int decode_list_t(glme_buf_t *dec, list_t *lst)
   {
     GLME_DECODE_STDDEF;
     GLME_DECODE_TYPE(dec, LIST_ID);
     GLME_DECODE_DELTA(dec);
     GLME_DECODE_STRUCT_PTR(dec, 0, lst->head, decode_link_t, link_t);
     GLME_DECODE_END(dec);
   }
```

