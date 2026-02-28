#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>
#include "../protocol/include/protocol.h"

#define PORT 8080
#define MAX_CLIENTS 2048

typedef struct {
  int fd;
  uint8_t *in;
  size_t in_len;
  size_t in_capacity;
  int have_header;
  ConveyHeader *header;
  size_t total_memory;
  uint8_t *out;
  size_t out_capacity;
  size_t out_len;
  size_t out_pos;
} Client;

int run_server() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(8080);

  if(inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0) return -1;

  if(bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) return -1;

  if(listen(fd, 128) < 0) return -1;

  printf("Listening on 127.0.0.1:8080...\n");

  return fd;
}

Client create_client(int fd) {
  uint8_t *in = malloc(4096);
  size_t in_capacity = 4096;
  size_t in_len = 0;

  Client client = {
    .fd = fd,
    .in = in,
    .in_capacity = in_capacity,
    .in_len = in_len,
    .have_header = -1,
    .header = NULL,
    .out = NULL,
    .out_pos = 0,
    .out_capacity = 0,
    .out_len = 0,
  };
}

void handle_request(ConveyFrame *f) {

  switch (f->header.type) {
    case MSG_AUTH_LOGIN: {
      MsgLogin l = {0};
      decode_payload_login(&f->payload, &l);

      break;
    }
  }
}

void handle_connection(Client client) {
  
}

void remove_client(int fd) {

}

int main() {
  int fd = run_server();
  nfds_t nfd = 2;
  struct pollfd fds[MAX_CLIENTS];
  fds[0].fd = STDIN_FILENO;
  fds[0].events = POLLIN;

  fds[1].fd = fd;
  fds[1].events = POLLIN;

  for(;;) { 
    int ret = poll(fds, nfd, -1);
    if(ret <= 0) {
      perror("Poll");
    }

    if(ret > 0) {
      if(fds[1].revents & POLLIN) {
        int cfd = accept(fd, NULL, NULL);
        if(cfd == -1) {
          printf("Error\n");
          continue;
        }

        if(nfd < MAX_CLIENTS) {
          fds[nfd].fd = cfd;
          fds[nfd].events = POLLIN;
          nfd++;
        }
      }
      else if(fds[0].revents & POLLIN) {
        continue;
      }

      for(int i = 2; i < nfd; i++) {
        if (fds[i].revents & (POLLHUP | POLLERR | POLLNVAL)) {
          fds[i].fd = -1;
          nfd--; 
        }
        else if(fds[i].revents & POLLIN) {
          handle_connection(fds[i].fd); 
        }
      }
    }
  }
}
