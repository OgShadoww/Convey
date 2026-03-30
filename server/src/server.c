#include <stdio.h>
#include <string.h>
#include <poll.h>
#include <unistd.h>

#include "../include/server.h"
#include "../include/server_net.h"
#include "../include/server_client.h"
#include "../include/server_handlers.h"

static void destroy_client_at(Server *server, nfds_t i) {
  if(i >= server->nfds) return;

  close(server->fds[i].fd);
  client_free(server->clients[i]);

  if (i != server->nfds - 1) {
    server->fds[i] = server->fds[server->nfds - 1];
    server->clients[i] = server->clients[server->nfds - 1];
  }

  server->fds[server->nfds - 1].fd = -1;
  server->fds[server->nfds - 1].events = 0;
  server->fds[server->nfds- 1].revents = 0;
  server->clients[server->nfds - 1] = NULL;

  server->nfds--;
}

int server_init(Server *server) {
  server->listen_fd = -1;
  server->tls_ctx = NULL;

  server->listen_fd = net_listen_tcp();
  if (server->listen_fd < 0) return -1;

  server->tls_ctx = net_tls_server_ctx_new();
  if (!server->tls_ctx) {
    close(server->listen_fd);
    server->listen_fd = -1;
    return -1;
  }

  server->nfds = 2;
  memset(server->fds, 0, sizeof(server->fds));
  memset(server->clients, 0, sizeof(server->clients));

  server->fds[0].fd = STDIN_FILENO;
  server->fds[0].events = POLLIN;

  server->fds[1].fd = server->listen_fd;
  server->fds[1].events = POLLIN;

  return 0;
}

int server_accept_one(Server *server) {
  Conn c = {0};
  if(net_tls_accept_client(server, &c) == 0) {
    if(server->nfds < MAX_CLIENTS) {
      server->fds[server->nfds].fd = c.fd;
      server->fds[server->nfds].events = POLLIN;
      server->fds[server->nfds].revents = 0;

      Client *newClient = client_new(c.fd, c.ssl);
      if(!newClient) {
        close(c.fd);
        SSL_shutdown(c.ssl);
        SSL_free(c.ssl);
        return -1;
      }
      server->clients[server->nfds] = newClient;
      (server->nfds)++;
      printf("Connection success\n");
      fflush(stdout);
    }
    else {
      close(c.fd);
      SSL_shutdown(c.ssl);
      SSL_free(c.ssl);
      return -1;
    }
  }

  return 0;
}

int server_run(Server *server) {
  for(;;) { 
    int ret = poll(server->fds, server->nfds, -1);
    if(ret < 0) {
      perror("Poll");
      continue;
    }

    if(ret > 0) {
      if(server->fds[1].revents & POLLIN) {
        server_accept_one(server);
      }

      for(nfds_t i = 2; i < server->nfds; i++) {
        if(server->fds[i].revents & (POLLHUP | POLLERR | POLLNVAL)) {
          printf("Destroying client : fd=%d\n", server->fds[i].fd);
          destroy_client_at(server, i);
          i--;
          continue;
        }
        if(server->fds[i].revents & POLLIN) {
          printf("Handling fd=%d\n", server->fds[i].fd);
          if(client_on_readable(server, server->clients[i]) < 0) {
            printf("Destroying client : fd=%d\n", server->fds[i].fd);
            destroy_client_at(server, i);
            i--;
            continue;
          }
        }
      }
    }
  }
  
  return 0;
}
