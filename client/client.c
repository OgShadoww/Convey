#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "../protocol/include/protocol.h"

int main() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(8080);

  if(inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0) return -1;

  if(connect(fd, &addr, sizeof(addr)) < 0) return -1;

  MsgLogin l = {0};
  strncpy(l.username, "Orest", sizeof(l.username) - 1);
  strncpy(l.password, "123", sizeof(l.password) - 1);

  // Compute exact payload size: u16 + username + u16 + password
  size_t ul = strlen(l.username);
  size_t pl = strlen(l.password);
  size_t payload_needed = 2 + ul + 2 + pl;

  Buff payload = {0};
  payload.data = (uint8_t*)malloc(payload_needed);
  if (!payload.data) {
    perror("malloc");
    return 1;
  }
  payload.len = payload_needed;
  payload.pos = 0;

  if (encode_payload_login(&payload, &l) < 0) {
    fprintf(stderr, "encode_payload_login failed\n");
    free(payload.data);
    return 1;
  }

  // Set payload.len to bytes actually used
  payload.len = payload.pos;
  payload.pos = 0;

  ConveyFrame f = {0};
  f.header.magic = CONVEY_MAGIC;
  f.header.version = CONVEY_VERSION;
  f.header.type = MSG_AUTH_LOGIN;
  f.payload = payload;

  if (write_frame(fd, &f) < 0) {
    fprintf(stderr, "write_frame failed\n");
    free_frame(&f);
    return 1;
  }

  free_frame(&f);
  return 0;
}
