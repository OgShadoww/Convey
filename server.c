#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <poll.h>

#define MAX_CLIENTS 64
#define PORT 8080

typedef struct {
  char *key;
  int value;
} MapEntry;

typedef struct {
  MapEntry *entries;
  int size;
  int capacity;
} Map;

static void die(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

Map *init_map() {
  Map *m = malloc(sizeof(Map));
  // Make error check
  m->size = 0;
  m->capacity = 64;
  m->entries = malloc(sizeof(MapEntry)*m->capacity);
  for(int i = 0; i < m->capacity; i++) {
    m->entries[i].key = NULL;
    m->entries[i].value = 0;  
  }

  return m;
}

int map_insert(Map *m, char *key, int value) {
  if(m->size >= m->capacity) {
    m->capacity *= 2;
    m->entries = realloc(m->entries, sizeof(MapEntry)*m->capacity);
    // Make error check    
  }
  
  m->entries[m->size].key = key;
  m->entries[m->size].value = value;
  m->size++;

  return 1;
}

int map_get(Map *m, char *key) {
  for(int i = 0; i < m->size; i++) {
    if(strcmp(key, m->entries[i].key) == 0) {
      return m->entries[i].value;
    }
  }

  return -1;
}

int main() {
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0); 
  if(socket_fd < 0) {
    die("Socket error");
  }

  struct sockaddr_in fd_adr;
  fd_adr.sin_family = AF_INET;
  fd_adr.sin_port = htons(8080);
  fd_adr.sin_addr.s_addr = INADDR_ANY;

  if(bind(socket_fd, (struct sockaddr*)&fd_adr, sizeof(fd_adr)) < 0) {
    die("Bind error");
  }
  if(listen(socket_fd, 10) < 0) {
    die("Listen error");
  }
  
  struct pollfd pfds[MAX_CLIENTS + 1];

  while(1) {
    //int ret = poll(pfds, );

    int client_fd = accept(socket_fd, NULL, NULL);
    if(client_fd < 0) {
      die("Client socket error");
    }

  }


  return 0;
}
