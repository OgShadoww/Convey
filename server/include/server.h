#ifndef SERVER_H
#define SERVER_H

#include <openssl/ssl.h>
#include <poll.h>

#define MAX_CLIENTS 2048

typedef struct Client Client;

typedef struct Server {
  int listen_fd;
  SSL_CTX *tls_ctx;
  nfds_t nfds;
  struct pollfd fds[MAX_CLIENTS];
  Client *clients[MAX_CLIENTS];
} Server;

int server_init(Server *s);
int server_run(Server *s);
int server_accept_one(Server *s);
void server_free(Server *s);

#endif
