#ifndef PROTOCOL_TRANSPORT_H
#define PROTOCOL_TRANSPORT_H

#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/socket.h>

// Transportation
ssize_t conn_read(int fd, void *buff, size_t n);
ssize_t conn_write(int fd, const void *buff, size_t n);

// Exact
int read_exact(int fd, void *buff, size_t len);
int write_all(int fd, const void *buff, size_t len);

#endif
