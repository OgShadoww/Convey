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

ssize_t conn_write(int fd, const void *buff, size_t n) {
  int r = write(fd, buff, n);
  if(r < 0) return -1;
  return r;
}

ssize_t conn_read(int fd, void *buff, size_t n) {
  int r = read(fd, buff, n);
  if(r < 0) return -1;

  return r;
}

int read_exact(int fd, void *buff, size_t len) {
  size_t total = 0;
  uint8_t *p = (uint8_t*)buff;

  while(total < len) {
    int n = conn_read(fd, p+total, len-total);

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
    int n = conn_write(fd, p+total, len-total);

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

int buff_write_u32(Buff *b, uint32_t v) {
  if(b->pos+4 > b->len) {
    return -1;
  }

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

// Testing
int main(void) {
  int fd = open("test.txt", O_WRONLY);

  // in
  ConveyHeader in = {
    .magic = CONVEY_MAGIC,
    .version = 0x01,
    .type = 0x69,
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
  ConveyHeader *out = malloc(sizeof(ConveyHeader));

  Buff *binary_out = malloc(sizeof(Buff));
  binary_out->data = malloc(sizeof(uint8_t)*CONVEY_HEADER_LEN);
  binary_out->pos = 0;
  binary_out->len = CONVEY_HEADER_LEN;
  read_exact(fd2, binary_out->data, CONVEY_HEADER_LEN);

  decode_header(binary_out, out);

  printf("In: %02X %d %d %d \n Out: %02X %d %d %d", in.magic, in.version, in.type, in.payload_len, out->magic, out->version, out->type, out->payload_len);

  free(binary_in->data);
  free(binary_in);
  free(binary_out->data);
  free(binary_out);
  free(out);
}
