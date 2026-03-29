#include "../include/server_net.h"
#include "../include/server.h"
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <unistd.h>
#include <stdio.h>

SSL_CTX *net_tls_server_ctx_new() {
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

int net_listen_tcp() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if(fd < 0) return -1;

  int opt = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if(bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    close(fd);
    return -1;
  }

  if(listen(fd, 128) < 0) {
    close(fd);
    return -1;
  }

  printf("Listening on 127.0.0.1:8080...\n");
  fflush(stdout);

  return fd;
}

int net_tls_accept_client(Server *s, Conn *c) {
  c->fd = accept(s->listen_fd, NULL, NULL);
  if(c->fd == -1) {
    printf("Error\n");
    return -1;
  }

  c->ssl = SSL_new(s->tls_ctx);
  if(!c->ssl) {
    ERR_print_errors_fp(stderr);
    close(c->fd);
    return -1;
  }

  if(SSL_set_fd(c->ssl, c->fd) != 1) {
    ERR_print_errors_fp(stderr);
    SSL_free(c->ssl);
    close(c->fd);
    return -1;
  }

  if(SSL_accept(c->ssl) <= 0) {
    ERR_print_errors_fp(stderr);
    SSL_free(c->ssl);
    close(c->fd);
    return -1;
  }

  return 0;
}
