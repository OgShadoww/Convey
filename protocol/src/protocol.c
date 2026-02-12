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
  int r = send(fd, buff, n, 0);

  return r;
}

ssize_t conn_read(int fd, void *buff, size_t n) {
  int r = read(fd, buff, n);
  if(r < 0) {
    perror("Exit failure");
    return -1;
  }

  return r;
}

int read_exact(int fd, void *buff, size_t len) {
  size_t total = 0;
  uint8_t *p = (uint8_t*)buff;

  while(total < len) {
    int n = conn_read(fd, p+total, len-total);
    total += n;
  }
  
  return 0;
}

int encode_header(Buff *b, ConveyHeader *h) {
  b->data[0] = h->magic>>24; 
  b->data[1] = h->magic>>16;
  b->data[2] = h->magic>>8;
  b->data[3] = h->magic>>0;

  b->data[4] = h->version;
  b->data[5] = h->type;
  b->data[6] = h->payload_len>>24;
  b->data[7] = h->payload_len>>16;
  b->data[8] = h->payload_len>>8;
  b->data[9] = h->payload_len>>0;

  return 1;
}

int main() {
  int fd = open("test.txt", O_RDONLY);
  ConveyHeader header = {
    CONVEY_MAGIC, 
    CONVEY_VERSION, 
    0x1, 
    0x128
  };
  Buff *b = malloc(sizeof(Buff));
  b->data = malloc(sizeof(uint8_t)*CONVEY_HEADER_LEN);
  b->len = CONVEY_HEADER_LEN;
  b->pos = 0;
  encode_header(b, &header);

  for(int i = 0; i < CONVEY_HEADER_LEN; i++) {
    write(STDOUT_FILENO, &b->data[i], sizeof(uint8_t));
  }
  write(STDOUT_FILENO, "\n", 1);
  
  free(b->data);
  free(b);
  close(fd);
}
