#include "../include/protocol_transport.h"
#include <stdint.h>

ssize_t conn_write(int fd, const void *buff, size_t n) {
  ssize_t r = send(fd, buff, n, 0);
  if(r < 0) return -1;
  return r;
}

ssize_t conn_read(int fd, void *buff, size_t n) {
  ssize_t r = recv(fd, buff, n, 0);
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

ssize_t read_some(int fd, void *buff, size_t len) {
  ssize_t n = conn_read(fd, (uint8_t*)buff, len);
  if(n <= 0) return -1;

  return n;
}

ssize_t write_some(int fd, const void *buff, size_t len) {
  ssize_t n = conn_write(fd, (uint8_t*)buff, len);
  if(n < 0) return -1;

  return n;
}
