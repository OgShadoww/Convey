#include "../include/server_client.h"
#include <poll.h>
#include <string.h>
#include <stdlib.h>

Client *client_new(int fd, SSL *ssl) {
  Client *client = NULL;
  Buff *in = NULL;
  Buff *out = NULL;
  ConveyHeader *header = NULL;

  in = malloc(sizeof(*in));
  if(!in) goto fail;
  in->len = 0;
  in->pos = 0;
  in->data = malloc(CONVEY_HEADER_LEN);
  if(!in->data) goto fail;

  out = malloc(sizeof(*out));
  if(!out) goto fail;
  out->len = 0;
  out->pos = 0;
  out->data = NULL;

  header = malloc(sizeof(*header));
  if(!header) goto fail;
  header->payload_len = 0;
  header->magic = CONVEY_MAGIC;
  header->type = 0;
  header->version = CONVEY_VERSION;

  client = malloc(sizeof(*client));
  if(!client) goto fail;

  client->fd = fd;
  client->ssl = ssl;
  client->in = in;
  client->state = CLIENT_RECV_HEADER;
  client->header = header;
  client->out = out;

  return client;

fail:
  if(in) {
    free(in->data);
    free(in);
  }
  if(out) {
    free(out->data);
    free(out);
  }
  free(header);
  if (client) free(client);
  return NULL;
}

void client_free(Client *client) { 
  if(!client) return;
  if(client->in) {
    free(client->in->data);
    free(client->in);
  }
  if(client->out) {
    free(client->out->data);
    free(client->out);
  }
  if(client->ssl) {
    SSL_shutdown(client->ssl);
    SSL_free(client->ssl);
    client->ssl = NULL;
  }
  free(client->header);
  free(client);
}
