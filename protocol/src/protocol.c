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

// Conn functions
ssize_t conn_write(int fd, const void *buff, size_t n) {
  int r = write(fd, buff, n);
  if(r < 0) return -1;
  return (ssize_t)r;
}

ssize_t conn_read(int fd, void *buff, size_t n) {
  int r = read(fd, buff, n);
  if(r < 0) return -1;

  return (ssize_t)r;
}

int read_exact(int fd, void *buff, size_t len) {
  size_t total = 0;
  uint8_t *p = (uint8_t*)buff;

  while(total < len) {
    ssize_t n = conn_read(fd, p+total, len-total);

    if(n == 0) return -1;
    if(n < 0) return -1;

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

int encode_header(Buff *b, ConveyHeader *h) {
  if(buff_write_u32(b, h->magic) == -1) return -1;
  if(buff_write_u8(b, h->version) == -1) return -1;
  if(buff_write_u8(b, h->type) == -1) return -1;
  if(buff_write_u32(b, h->payload_len) == -1) return -1;

  return 0;
}

int decode_header(Buff *b, ConveyHeader *h) {
  buff_read_u32(b, &h->magic);
  buff_read_u8(b, &h->version);
  buff_read_u8(b, &h->type);
  buff_read_u32(b, &h->payload_len);

  return 0;
}

int decode_payload_login(Buff *b, MsgLogin *p) {
  uint16_t ul = 0, pl = 0;
  if(buff_read_u16(b, &ul) == -1) return -1;

  if(ul >= sizeof(p->username)) return -1;

  for(int i = 0; i < (int)ul; i++) {
    uint8_t ch;
    buff_read_u8(b, &ch);

    p->username[i] = (char)ch;
  }
  p->username[ul] = '\0';

  if(buff_read_u16(b, &pl) == -1) return -1;

  if(ul >= sizeof(p->password)) return -1;

  for(int i = 0; i < pl; i++) {
    uint8_t ch;
    buff_read_u8(b, &ch);

    p->password[i] = (char)ch;
  }
  p->password[ul] = '\0';

  return 0;
}

int encode_payload_login(Buff *b, MsgLogin *p) {
  uint16_t ul = strlen(p->username);
  uint16_t pl = strlen(p->password);
  buff_write_u16(b, ul);
  for(int i = 0; i < ul; i++) {
    buff_write_u8(b, (uint8_t)p->username[i]);
  }
  buff_write_u16(b, pl);
  for(int i = 0; i < pl; i++) {
    buff_write_u8(b, (uint8_t)p->password[i]);
  }

  return 0;
}

int read_frame(int fd, ConveyFrame *f) {
  // Read header
  uint8_t header_bytes[CONVEY_HEADER_LEN];
  if(read_exact(fd, header_bytes, CONVEY_HEADER_LEN) == -1) return -1;

  Buff b = {.data = header_bytes, .len = CONVEY_HEADER_LEN, .pos = 0};
  if(decode_header(&b, &f->h) == -1) return -1;

  if(f->h.payload_len > CONVEY_MAX_PAYLOAD) return -1;

  f->payload.data = malloc(f->h.payload_len);
  if(!f->payload.data) return -1;
  f->payload.len = f->h.payload_len;
  f->payload.pos = 0;

  if(read_exact(fd, f->payload.data, f->h.payload_len) == -1) return -1;

  return 0;
}

int write_frame(int fd, ConveyFrame *f) {
  // Write header
  uint8_t header_bytes[CONVEY_HEADER_LEN];
  Buff hb = {.data = header_bytes, .len = CONVEY_HEADER_LEN, .pos = 0};

  f->h.payload_len = (uint32_t)f->payload.len;

  if(encode_header(&hb, &f->h) == -1) return -1;
  if(write_all(fd, header_bytes, CONVEY_HEADER_LEN) == -1) return -1;

  // Write payload
  if(f->payload.len > 0) {
    if(!f->payload.data) return -1;
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

// Testing
int main(void) {
  int fd = open("test.txt", O_WRONLY);

  // in
  ConveyHeader in = {
    .magic = CONVEY_MAGIC,
    .version = 0x01,
    .type = MSG_OK,
    .payload_len = CONVEY_HEADER_LEN
  };
  Buff *binary_in = malloc(sizeof(Buff));
  binary_in->data = malloc(sizeof(uint8_t)*CONVEY_HEADER_LEN);
  binary_in->pos = 0;
  binary_in->len = CONVEY_HEADER_LEN;
  encode_header(binary_in, &in);
  write_all(fd, binary_in->data, CONVEY_HEADER_LEN);
  close(fd);

  int fd2 = open("test.txt", O_RDONLY);
  // Out
  int out = open("test.txt", O_RDONLY);
  ConveyFrame f;
  read_frame(out, &f);

  printf("%02X %d %d %d", f.h.magic, f.h.version, f.h.type, f.h.payload_len);

  

  free(binary_in->data);
  free(binary_in);
}
