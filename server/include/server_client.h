#ifndef SERVER_CLIENT_H
#define SERVER_CLIENT_H

#include <openssl/ssl.h>
#include "protocol.h"

typedef struct Server Server;
#define MAX_CLIENTS 2048

typedef enum {
  CLIENT_RECV_HEADER = 0,
  CLIENT_RECV_PAYLOAD = 1,
} ClientRecvState;

typedef struct Client{
  int fd;
  SSL *ssl;
  Buff *in;
  ClientRecvState state;
  ConveyHeader *header;
  size_t total_memory;
  Buff *out;
} Client;

Client *client_new(int fd, SSL *sll);
void client_free(Client *c);
int client_on_readable(Client *c, Server *s);

#endif
