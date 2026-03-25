#include "../include/protocol_transport.h"

ssize_t conn_write(SSL *ssl, int fd, const void *buff, size_t n) {
  if(ssl) {
    ssize_t r = SSL_write(ssl, buff, n);
    if(r <= 0) return -1;
    return r;
  }

  ssize_t r = send(fd, buff, n, 0);
  if(r <= 0) return -1;
  return r;
}

ssize_t conn_read(SSL *ssl, int fd, void *buff, size_t n) {
  if(ssl) {
    ssize_t r = SSL_read(ssl, buff, n);
    if(r <= 0) return -1;
    return r;
  }
  ssize_t r = recv(fd, buff, n, 0);
  if(r <= 0) return -1;

  return r;
}

int read_exact(int fd, void *buff, size_t len) {
  size_t total = 0;
  uint8_t *p = (uint8_t*)buff;

  while(total < len) {
    ssize_t n = conn_read(NULL, fd, p+total, len-total);
    if(n <= 0) return -1;
    total += n;
  }
  
  return 0;
}

int write_all(int fd, const void *buff, size_t len) {
  size_t total = 0;
  uint8_t *p = (uint8_t*)buff;

  while(total < len) {
    ssize_t n = conn_write(NULL, fd, p+total, len-total);
    if (n <= 0) return -1;
    total += (size_t)n;
  }

  return 0;
}

ssize_t read_some(SSL *ssl, int fd, void *buff, size_t len) {
  return conn_read(ssl, fd, (uint8_t*)buff, len);
}

ssize_t write_some(SSL *ssl, int fd, const void *buff, size_t len) {
  return conn_write(ssl, fd, (uint8_t*)buff, len);
}
