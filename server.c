#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

typedef struct {
  char *key;
  int value;
} MapItem;

typedef struct {
  MapItem *items;
  int size;
  int capacity;
} Map;

void *handle_connection(void *arg) {
  int client_fd = *(int*)arg;
  free(arg);

  send(client_fd, "Please write your nickname", 26, 0);

  return 0;
}

int main() {
  int socket_fd = socket(PF_LOCAL, SOCK_STREAM, 0);
  if(socket_fd < 0) {
    perror("Socket error");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in fd_adr;
  fd_adr.sin_family = AF_INET;
  fd_adr.sin_port = htons(6060);
  fd_adr.sin_addr.s_addr = INADDR_ANY;

  if(bind(socket_fd, (struct sockaddr*)&fd_adr, sizeof(fd_adr)) < 0) {
    perror("Bind error");
    exit(EXIT_FAILURE);
  }
  if(listen(socket_fd, 10) < 0) {
    perror("Listen error");
    exit(EXIT_FAILURE);
  }

  while(1) {
    pthread_t t;
    int client_fd = accept(socket_fd, NULL, NULL);
    if(client_fd < 0) {
      perror("Client fd error");
      exit(EXIT_FAILURE);
    }

    int *pdf = malloc(sizeof(int));
    *pdf = client_fd;

    pthread_create(&t, NULL, handle_connection, pdf);
  }

  return 0;
}
