#include <stdio.h>
#include <string.h>
#include <poll.h>
#include <unistd.h>

#include "../include/server.h"
#include "../include/server_net.h"
#include "../include/server_client.h"
#include "../include/server_handlers.h"

static void destroy_client_at(Server *s, nfds_t i) {
  if(i >= s->nfds) return;

  close(s->fds[i].fd);
  client_free(s->clients[i]);

  if (i != s->nfds - 1) {
    s->fds[i] = s->fds[s->nfds - 1];
    s->clients[i] = s->clients[s->nfds - 1];
  }

  s->fds[s->nfds - 1].fd = -1;
  s->fds[s->nfds - 1].events = 0;
  s->fds[s->nfds- 1].revents = 0;
  s->clients[s->nfds - 1] = NULL;

  s->nfds--;
}

int server_init(Server *s) {
  s->listen_fd = -1;
  s->tls_ctx = NULL;

  s->listen_fd = net_listen_tcp();
  if (s->listen_fd < 0) return -1;

  s->tls_ctx = net_tls_server_ctx_new();
  if (!s->tls_ctx) {
    close(s->listen_fd);
    s->listen_fd = -1;
    return -1;
  }

  s->nfds = 2;
  memset(s->fds, 0, sizeof(s->fds));
  memset(s->clients, 0, sizeof(s->clients));

  s->fds[0].fd = STDIN_FILENO;
  s->fds[0].events = POLLIN;

  s->fds[1].fd = s->listen_fd;
  s->fds[1].events = POLLIN;

  return 0;
}

int server_accept(Server *s) {
  Conn c = {0};
  if(net_tls_accept_client(s, &c) == 0) {
    if(s->nfds < MAX_CLIENTS) {
      s->fds[s->nfds].fd = c.fd;
      s->fds[s->nfds].events = POLLIN;
      s->fds[s->nfds].revents = 0;

      Client *newClient = client_new(c.fd, c.ssl);
      if(!newClient) {
        close(c.fd);
        SSL_shutdown(c.ssl);
        SSL_free(c.ssl);
        return -1;
      }
      s->clients[s->nfds] = newClient;
      (s->nfds)++;
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

int server_run(Server *s) {
  for(;;) { 
    int ret = poll(s->fds, s->nfds, -1);
    if(ret < 0) {
      perror("Poll");
      continue;
    }

    if(ret > 0) {
      if(s->fds[1].revents & POLLIN) {
        server_accept(s);
      }

      for(nfds_t i = 2; i < s->nfds; i++) {
        if(s->fds[i].revents & (POLLHUP | POLLERR | POLLNVAL)) {
          printf("Destroying client : fd=%d\n", s->fds[i].fd);
          destroy_client_at(s, i);
          i--;
          continue;
        }
        if(s->fds[i].revents & POLLIN) {
          printf("Handling fd=%d\n", s->fds[i].fd);
          if(handle_connection(s->clients[i]) < 0) {
            printf("Destroying client : fd=%d\n", s->fds[i].fd);
            destroy_client_at(s, i);
            i--;
            continue;
          }
        }
      }
    }
  }
  
  return 0;
}
