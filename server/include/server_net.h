#ifndef SERVER_NET_H
#define SERVER_NET_H

#include <openssl/ssl.h>

#define PORT 8080

typedef struct Server Server;

typedef struct {
  int fd;
  SSL *ssl;
} Conn;

int net_listen_tcp();
int net_tls_accept_client(Server *s, Conn *c);
SSL_CTX *net_tls_server_ctx_new();
void net_tls_server_ctx_free(SSL_CTX *ctx);

#endif
