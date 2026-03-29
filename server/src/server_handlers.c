#include <stdio.h>
#include <stdlib.h>
#include "../include/server_client.h"
#include "../include/server_handlers.h"

int handle_auth_login(Client *client) {
  printf("Client handle\n");
  fflush(stdout);
  MsgLogin *l = malloc(sizeof(*l));
  if(!l) {
    perror("Malloc failled");
    return -1;
  }
  decode_payload_login(client->in, l);
  printf("username: %s password: %s", l->username, l->password);
  fflush(stdout);

  free(l);

  // Clear buffers
  free(client->in->data);
  free(client->in);

  client->in = malloc(sizeof(*client->in));
  if(!client->in) return -1;

  client->in->data = malloc(CONVEY_HEADER_LEN);
  if(!client->in->data) {
    free(client->in);
    client->in = NULL;
    return -1;
  }

  client->in->len = 0;
  client->in->pos = 0;
  client->state = CLIENT_RECV_HEADER;
  return 0;
}


int handle_request(Client *client) {
  switch (client->header->type) {
    case MSG_AUTH_LOGIN: {
      return handle_auth_login(client);
    }
    default: {
      return -1;
    }
  }
}

int handle_connection(Client *client) {
  if(client->state == CLIENT_RECV_HEADER) {
    int n = read_some(client->ssl, client->fd, client->in->data + client->in->len, CONVEY_HEADER_LEN - client->in->len);
    if(n <= 0) return -1;
    client->in->len += n;
    if(client->in->len == CONVEY_HEADER_LEN) {
      client->state = CLIENT_RECV_PAYLOAD;
      decode_header(client->in, client->header);
      free(client->in->data);
      free(client->in);

      // Clean in buffer for payload
      client->in = malloc(sizeof(*client->in));
      if(!client->in) return -1;

      client->in->data = malloc(client->header->payload_len);
      if(!client->in->data) {
        free(client->in);
        client->in = NULL;
        return -1;
      }
      client->in->pos = 0;
      client->in->len = 0;
    }
    return 0;
  }
  if(client->state == CLIENT_RECV_PAYLOAD) {
    int n = read_some(client->ssl, client->fd, client->in->data + client->in->len, client->header->payload_len - client->in->len);

    if(n <= 0) return -1;
    client->in->len += n;
    if(client->in->len == client->header->payload_len) {
      return handle_request(client);
    }
  }

  return 0;
}
