#ifndef PROTOCOL_TRANSPORT_H
#define PROTOCOL_TRANSPORT_H

#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/socket.h>
#include <openssl/ssl.h>

// Transportation
ssize_t conn_read(SSL *ssl, int fd, void *buff, size_t n);
ssize_t conn_write(SSL *ssl, int fd, const void *buff, size_t n);

// Exact | Testing only
int read_exact(int fd, void *buff, size_t len);
int write_all(int fd, const void *buff, size_t len);

// Reading some | To have non-blocking concurrency
ssize_t read_some(SSL *ssl, int fd, void *buff, size_t len);
ssize_t write_some(SSL *ssl, int fd, const void *buff, size_t len);

#endif
