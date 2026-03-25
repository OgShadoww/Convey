#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "../protocol/include/protocol.h"

#define PORT 8080
#define MAX_CLIENTS 2048

typedef struct {
  int fd;
  SSL *ssl;
} Conn;

typedef struct {
  int fd;
  SSL *ssl;
  Buff *in;
  int have_header;
  ConveyHeader *header;
  size_t total_memory;
  Buff *out;
} Client;

SSL_CTX *create_server_ctx() {
  SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
  if (!ctx) {
    ERR_print_errors_fp(stderr);
    return NULL;
  }

  if(SSL_CTX_use_certificate_file(ctx, "certs/cert.pem", SSL_FILETYPE_PEM) <= 0) {
    ERR_print_errors_fp(stderr);
    SSL_CTX_free(ctx);
    return NULL;
  }
  
  if(SSL_CTX_use_PrivateKey_file(ctx, "certs/key.pem", SSL_FILETYPE_PEM) <= 0) {
    ERR_print_errors_fp(stderr);
    SSL_CTX_free(ctx);
    return NULL;
  }

  return ctx;
}

int run_server() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if(fd < 0) return -1;

  int opt = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if(bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) return -1;

  if(listen(fd, 128) < 0) return -1;

  printf("Listening on 127.0.0.1:8080...\n");
  fflush(stdout);

  return fd;
}

Client *create_client(int fd, SSL *ssl) {
  Buff *in = malloc(sizeof(*in));
  if(!in) goto fail;
  in->len = 0;
  in->pos = 0;
  in->data = malloc(CONVEY_HEADER_LEN);
  if(!in->data) goto fail;

  Buff *out = malloc(sizeof(*out));
  if(!out) goto fail;
  out->len = 0;
  out->pos = 0;
  out->data = NULL;

  ConveyHeader *header = malloc(sizeof(*header));
  if(!header) goto fail;
  header->payload_len = 0;
  header->magic = CONVEY_MAGIC;
  header->type = 0;
  header->version = CONVEY_VERSION;

  Client *client = malloc(sizeof(*client));
  if(!client) goto fail;

  client->fd = fd;
  client->ssl = ssl;
  client->in = in;
  client->have_header = 0;
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
  free(client);
  SSL_shutdown(client->ssl);
  SSL_free(client->ssl);
  return NULL;
}

void remove_client(Client *client) { 
  if(!client) return;
  if(client->ssl) {
    SSL_shutdown(client->ssl);
    SSL_free(client->ssl);
  }
  if(client->in) {
    free(client->in->data);
    free(client->in);
  }
  if(client->out) {
    free(client->out->data);
    free(client->out);
  }
  free(client->header);
  free(client);
}

void handle_request(Client *client) {

  switch (client->header->type) {
    case MSG_AUTH_LOGIN: {
      printf("Client handle\n");
      fflush(stdout);
      MsgLogin *l = malloc(sizeof(*l));
      if(!l) {
        perror("Malloc failled");
        return;
      }
      decode_payload_login(client->in, l);
      printf("%s %s\n", l->username, l->password);
  
      free(l);

      // Clear buffers
      free(client->in->data);
      free(client->in);

      client->in = malloc(sizeof(*client->in));
      if(!client->in) return;

      client->in->data = malloc(CONVEY_HEADER_LEN);
      if(!client->in->data) return;

      client->in->len = 0;
      client->in->pos = 0;
      client->have_header = 0;
      break;
    }
    default: {
      printf("Connected default\n");
    }
  }
}

int handle_connection(Client *client) {
  if(!client->have_header) {
    int n = read_some(client->ssl, client->fd, client->in->data + client->in->len, CONVEY_HEADER_LEN - client->in->len);
    if(n <= 0) return -1;
    client->in->len += n;
    if(client->in->len == CONVEY_HEADER_LEN) {
      client->have_header = 1;
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
  if(client->have_header) {
    int n = read_some(client->ssl, client->fd, client->in->data + client->in->len, client->header->payload_len - client->in->len);

    if(n < 0) return -1;
    client->in->len += n;
    if(client->in->len == client->header->payload_len) {
      handle_request(client);
    }
    return 0;
  }

  return 0;
}

void main_loop() {

}

int main() {
  int fd = run_server();
  if(fd < 0) return -1;

  SSL_CTX *ctx = create_server_ctx();
  if(!ctx) return -1;

  // Poll concurrency
  nfds_t nfd = 2;
  struct pollfd fds[MAX_CLIENTS] = {0};
  fds[0].fd = STDIN_FILENO;
  fds[0].events = POLLIN;

  fds[1].fd = fd;
  fds[1].events = POLLIN;

  // Array for client
  Client *clients[MAX_CLIENTS] = {0};

  for(;;) { 
    int ret = poll(fds, nfd, -1);
    if(ret <= 0) {
      perror("Poll");
      continue;
    }

    if(ret > 0) {
      if(fds[1].revents & POLLIN) {
        int cfd = accept(fd, NULL, NULL);
        if(cfd == -1) {
          printf("Error\n");
          continue;
        }

        SSL *ssl = SSL_new(ctx);
        if(!ssl) {
          ERR_print_errors_fp(stderr);
          close(cfd);
          continue;
        }

        if(SSL_set_fd(ssl, cfd) != 1) {
          ERR_print_errors_fp(stderr);
          SSL_free(ssl);
          close(cfd);
          continue;
        }

        if(SSL_accept(ssl) <= 0) {
          ERR_print_errors_fp(stderr);
          SSL_free(ssl);
          close(cfd);
          continue;
        }

        if(nfd < MAX_CLIENTS) {
          fds[nfd].fd = cfd;
          fds[nfd].events = POLLIN;
          fds[nfd].revents = 0;

          Client *newClient = create_client(cfd, ssl);
          if(!newClient) {
            SSL_shutdown(ssl);
            SSL_free(ssl);
            close(cfd);
            continue;
          }
          clients[nfd] = newClient;
          nfd++;
          printf("Connection success\n");
        }
        else {
          close(cfd);
        }
      }

      for(nfds_t i = 2; i < nfd; i++) {
        if(fds[i].revents & (POLLHUP | POLLERR | POLLNVAL)) {
          close(fds[i].fd);
          printf("Remove client %d\n", fds[i].fd);
          remove_client(clients[i]);
          clients[i] = NULL;
          
          if (i != nfd - 1) {
              fds[i] = fds[nfd - 1];
              clients[i] = clients[nfd - 1];
          }

          fds[nfd - 1].fd = -1;
          fds[nfd - 1].events = 0;
          fds[nfd - 1].revents = 0;
          clients[nfd - 1] = NULL;
          nfd--;

          i--;
          continue;
        }
        if(fds[i].revents & POLLIN) {
          printf("Handling fd=%d\n", fds[i].fd);
          if(handle_connection(clients[i]) < 0) {
            close(fds[i].fd);
            remove_client(clients[i]);
            fds[i].fd = -1;
            fds[i].events = 0;
            fds[i].revents = 0;
          }
        }
      }
    }
  }

  SSL_CTX_free(ctx);
}
