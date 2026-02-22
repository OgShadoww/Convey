#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include "../include/protocol.h"

// ===============================
// BufferLOW-LEVEL CONNECTION I/O
// ===============================

ssize_t conn_write(int fd, const void *buff, size_t n) {
  ssize_t r = write(fd, buff, n);
  if(r < 0) return -1;
  return r;
}

ssize_t conn_read(int fd, void *buff, size_t n) {
  ssize_t r = read(fd, buff, n);
  if(r < 0) return -1;

  return r;
}

int read_exact(int fd, void *buff, size_t len) {
  size_t total = 0;
  uint8_t *p = (uint8_t*)buff;

  while(total < len) {
    ssize_t n = conn_read(fd, p+total, len-total);
    if(n <= 0) return -1;
    total += n;
  }
  
  return 0;
}

int write_all(int fd, const void *buff, size_t len) {
  size_t total = 0;
  uint8_t *p = (uint8_t*)buff;

  while(total < len) {
    ssize_t n = conn_write(fd, p+total, len-total);
    if (n <= 0) return -1;
    total += (size_t)n;
  }

  return 0;
}

// ===============================
// BUFFER OPERATIONS
// ===============================

// Write Operations
int buff_write_u8(Buff *b, uint8_t v) {
  if(b->pos + 1 > b->len) return -1;
  b->data[b->pos++] = v;

  return 0;
}

int buff_write_u16(Buff *b, uint16_t v) {
  if(b->pos + 2 > b->len) return -1;

  b->data[b->pos+0] = (uint8_t)((v >> 8) & 0xFF);
  b->data[b->pos+1] = (uint8_t)((v >> 0) & 0xFF);

  b->pos += 2;
  return 0;
}

int buff_write_u32(Buff *b, uint32_t v) {
  if(b->pos+4 > b->len) return -1;

  b->data[b->pos+0] = (uint8_t)((v>>24) & 0xFF);
  b->data[b->pos+1] = (uint8_t)((v>>16) & 0xFF);
  b->data[b->pos+2] = (uint8_t)((v>>8) & 0xFF);
  b->data[b->pos+3] = (uint8_t)((v>>0) & 0xFF);

  b->pos += 4;
  return 0;
}

// Read Operations
int buff_read_u8(Buff *b, uint8_t *out) {
  if(b->pos + 1 > b->len) return -1;
  *out = b->data[b->pos++];

  return 0;
}

int buff_read_u16(Buff *b, uint16_t *out) {
  if(b->pos + 2 > b->len) return -1;

  uint16_t p = 0;
  p |= (uint16_t)b->data[b->pos+0] << 8;
  p |= (uint16_t)b->data[b->pos+1] << 0;

  *out = p;
  b->pos += 2;

  return 0;
}

int buff_read_u32(Buff *b, uint32_t *out) {
  if(b->pos + 4 > b->len) return -1;

  uint32_t p = 0;
  p |= (uint32_t)b->data[b->pos+0] << 24;
  p |= (uint32_t)b->data[b->pos+1] << 16;
  p |= (uint32_t)b->data[b->pos+2] << 8;
  p |= (uint32_t)b->data[b->pos+3] << 0;

  *out = p;
  b->pos += 4;

  return 0;
}
// ===============================
// FRAME HEADER OPERATIONS
// ===============================

int encode_header(Buff *b, ConveyHeader *h) {
  if(buff_write_u32(b, h->magic) == -1) return -1;
  if(buff_write_u8(b, h->version) == -1) return -1;
  if(buff_write_u8(b, h->type) == -1) return -1;
  if(buff_write_u32(b, h->payload_len) == -1) return -1;

  return 0;
}

int decode_header(Buff *b, ConveyHeader *h) {
  if(buff_read_u32(b, &h->magic) == -1)  return -1;
  if(buff_read_u8(b, &h->version) == -1) return -1;
  if(buff_read_u8(b, &h->type) == -1) return -1;
  if(buff_read_u32(b, &h->payload_len) == -1) return -1;

  return 0;
}

// ===============================
// MESSAGE PAYLOAD OPERATIONS
// ===============================

// Login decode/encode
int decode_payload_login(Buff *b, MsgLogin *p) {
  uint16_t ul = 0, pl = 0;
  if(buff_read_u16(b, &ul) == -1) return -1;

  if(ul >= sizeof(p->username)+1) return -1;

  for(int i = 0; i < (int)ul; i++) {
    uint8_t ch;
    if(buff_read_u8(b, &ch) == -1) return -1;

    p->username[i] = (char)ch;
  }
  p->username[ul] = '\0';

  if(buff_read_u16(b, &pl) == -1) return -1;

  if(pl >= sizeof(p->password)+1) return -1;

  for(int i = 0; i < pl; i++) {
    uint8_t ch;
    if(buff_read_u8(b, &ch) == -1) return -1;

    p->password[i] = (char)ch;
  }
  p->password[pl] = '\0';

  return 0;
}

int encode_payload_login(Buff *b, MsgLogin *p) {
  uint16_t ul = strlen(p->username);
  uint16_t pl = strlen(p->password);

  if (b->pos + (size_t)(2 + ul + 2 + pl) > b->len) return -1;

  if(buff_write_u16(b, ul) == -1) return -1;
  for(int i = 0; i < ul; i++) {
    if(buff_write_u8(b, (uint8_t)p->username[i]) == -1) return -1;
  }
  if(buff_write_u16(b, pl) == -1) return -1;
  for(int i = 0; i < pl; i++) {
    if(buff_write_u8(b, (uint8_t)p->password[i]) == -1) return -1;
  }

  return 0;
}

// OK 
int send_ok(int fd) {
  ConveyFrame f = { 
    .header = {
      .magic = CONVEY_MAGIC,
      .version = CONVEY_VERSION,
      .type = MSG_OK,
      .payload_len = 0
    },
    .payload = { 
      .data = NULL,
      .pos = 0,
      .len = 0
    }
  };

  write_frame(fd, &f);

  return 0;
}

// Error

char *ConveyErrorMessage[] = {
  [ERR_WRONG_TYPE] = "Hello world",
};

char* convey_error_str(ErrorTypes type) {
  if(ConveyErrorMessage[type] != NULL) {
    return ConveyErrorMessage[type];
  }
  
  return NULL;
}

int decode_payload_error(Buff *b, MsgError *e) {
  // Reading the type of error
  uint8_t type = 0;
  if(buff_read_u8(b, &type) == -1) return -1;
  e->error_type = type;

  // Reading the message
  uint16_t len = 0;
  if(buff_read_u16(b, &len) == -1) return -1;
  
  for(int i = 0; i < len; i++) {
    uint8_t ch = 0;

    if(buff_read_u8(b, &ch) == -1) return -1;
    e->error_message[i] = (char)ch;
  }

  return 0;
}

int encode_payload_error(Buff *b, MsgError *e) {
  // Writing type
  if(buff_write_u8(b, e->error_type) == -1) return -1;

  // Writing message
  uint16_t len = strlen(e->error_message);
  if(buff_write_u16(b, len) == -1) return -1;
  for(int i = 0; i < len; i++) {
    if(buff_write_u8(b, e->error_message[i]) == -1) return -1;
  }

  return 0;
}

int send_error(int fd, ErrorTypes type) {
  uint32_t payload_len = sizeof(uint8_t) + sizeof(uint16_t) + strlen(convey_error_str(type));

  MsgError e = { 
    .error_type = type,
    .error_message = convey_error_str(type)
  };
  Buff p; 
  if(encode_payload_error(&p, &e) == -1) return -1;

  ConveyFrame f = {
    .header = {
      .magic = CONVEY_MAGIC,
      .version = CONVEY_VERSION,
      .type = MSG_ERROR,
      .payload_len = payload_len
    },
    .payload = p
  };

  return 0;
}

// ===============================
// FRAME OPERATIONS READ / WRITE
// ===============================

int read_frame(int fd, ConveyFrame *f) {
  // Read header
  uint8_t header_bytes[CONVEY_HEADER_LEN];
  if(read_exact(fd, header_bytes, CONVEY_HEADER_LEN) == -1) return -1;

  Buff b = {.data = header_bytes, .len = CONVEY_HEADER_LEN, .pos = 0};
  if(decode_header(&b, &f->header) == -1) return -1;

  if(f->header.magic != CONVEY_MAGIC || f->header.payload_len > CONVEY_MAX_PAYLOAD || f->header.version != CONVEY_VERSION) return -1;

  if(f->header.payload_len > 0) {
    f->payload.data = malloc(f->header.payload_len);
    if(f->payload.data == NULL) return -1;
  }
  else {
    f->payload.data = NULL;
  }
  f->payload.len = f->header.payload_len;
  f->payload.pos = 0;

  if(read_exact(fd, f->payload.data, f->header.payload_len) == -1) {
    if(f->payload.len > 0) {
      free(f->payload.data);
    }
    return -1;
  }

  return 0;
}

int write_frame(int fd, ConveyFrame *f) {
  // Write header
  uint8_t header_bytes[CONVEY_HEADER_LEN];
  Buff hb = {.data = header_bytes, .len = CONVEY_HEADER_LEN, .pos = 0};

  f->header.payload_len = (uint32_t)f->payload.len;

  if(encode_header(&hb, &f->header) == -1) return -1;
  if(write_all(fd, header_bytes, CONVEY_HEADER_LEN) == -1) return -1;

  // Write payload
  if(f->payload.len > 0) {
    if(f->payload.data == NULL) return -1;
    if(write_all(fd, f->payload.data, f->payload.len) == -1) return -1;
  }

  return 0;
}

void free_frame(ConveyFrame *f) {
  free(f->payload.data);
  f->payload.data = NULL;
  f->payload.len = 0;
  f->payload.pos = 0;
}
