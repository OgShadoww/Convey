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
  Buff *in;
  int have_header;
  ConveyHeader *header;
  size_t total_memory;
  Buff *out;
} Client;

int run_server() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);

  if(inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0) return -1;

  if(bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) return -1;

  if(listen(fd, 128) < 0) return -1;

  printf("Listening on 127.0.0.1:8080...\n");

  return fd;
}

Client *create_client(int fd) {
  size_t in_capacity = 4096;
  size_t in_len = 0;
  Buff *in = malloc(sizeof(Buff));
  in->len = 0;
  in->pos = 0;
  in->data = NULL;

  Buff *out = malloc(sizeof(Buff));
  in->len = 0;
  in->pos = 0;
  in->data = NULL;

  ConveyHeader *header = malloc(sizeof(ConveyHeader));
  header->payload_len = 0;
  header->magic = CONVEY_HEADER_LEN;
  header->type = 0;
  header->version = CONVEY_VERSION;

  Client *client = malloc(sizeof(Client));
  client->fd = fd;
  client->in = in;
  client->have_header = -1;
  client->header = header;
  client->out = out;

  return client;
}

void remove_client(Client *client) { 
  free(client->in->data);
  free(client->in);
  free(client->out->data);
  free(client->out);
  free(client->header);
  free(client);
}

void handle_request(Client *client) {

  switch (client->header->type) {
    case MSG_AUTH_LOGIN: {
      MsgLogin l = {0};
      decode_payload_login(client->in, &l);

      break;
    }
  }
}

int handle_connection(Client *client) {
  if(!client->have_header) {
    size_t n = read_some(client->fd, client->in, CONVEY_HEADER_LEN - client->in->len);
    if(n < 0) return -1;
    client->in->pos += n;
    if(client->in->pos == CONVEY_HEADER_LEN) {
      client->have_header = 1;
      decode_header(client->in, client->header);
      free(client->in->data);
      free(client->in);

      // Clean in buffer for payload
      client->in = malloc(sizeof(Buff));
      client->in->data = malloc(client->header->payload_len);
      client->in->pos = 0;
      client->in->len = 0;
    }
    if(client->have_header) {
      size_t n = read_some(client->fd, client->in->data, client->header->payload_len - client->in->len);

      if(n < 0) return -1;
      client->in->len += n;
      if(client->in->len == client->header->payload_len) {
        handle_request(client);
      }
    }
  }
}

int main() {
  int fd = run_server();

  // Poll concurrency
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
          Client *newClient = create_client(cfd);
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
