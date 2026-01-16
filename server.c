#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <poll.h>
#include <fcntl.h>
#include "uttils.h"

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

int map_get_val(Map *m, char *key) {
  for(int i = 0; i < m->size; i++) {
    if(strcmp(key, m->entries[i].key) == 0) {
      return m->entries[i].value;
    }
  }

  return -1;
}

char *map_get_key(Map *m, int value) {
  for(int i = 0; i < m->size; i++) {
    if(value == m->entries[i].value) {
      return m->entries[i].key;
    }
  }

  return NULL;
}


int main() {
  // Users map
  Map *Users = init_map();

  int socket_fd = socket(AF_INET, SOCK_STREAM, 0); 
  if(socket_fd < 0) {
    die("Socket error");
  }

  struct sockaddr_in fd_adr;
  fd_adr.sin_family = AF_INET;
  fd_adr.sin_port = htons(8080);
  fd_adr.sin_addr.s_addr = INADDR_ANY;

  if(bind(socket_fd, (struct sockaddr*)&fd_adr, sizeof(fd_adr)) < 0) die("Bind error");
  if(listen(socket_fd, 10) < 0) die("Listen error");
  
  struct pollfd pfds[MAX_CLIENTS + 2];
  int nfds = MAX_CLIENTS + 2;

  // Listening socket init
  pfds[0].fd = socket_fd;
  pfds[0].events = POLLIN;

  // Admin stdin
  pfds[1].fd = STDIN_FILENO;
  pfds[1].events = POLLIN;

  // Init clinets sockets
  for(int i = 2; i < MAX_CLIENTS + 2; i++) {
    pfds[i].fd = -1;
    pfds[i].events = POLLIN;
  }

  while(1) {
    write(STDOUT_FILENO, "> ", 2);
    int ret = poll(pfds, nfds, -1);
    if(ret < 0) die("Poll error");
    
    // Admin panel
    if(pfds[1].revents & POLLIN) {
      printf("Please write the command for the admin: ");
      char buffer[256];
      size_t n = read(STDIN_FILENO, buffer, 256);

      if(n > 0) {
        write(STDOUT_FILENO, buffer, n);
      }

      continue;
    }
    // Listening socket check
    else if(pfds[0].revents & POLLIN) {
      int client_fd = accept(socket_fd, NULL, NULL);
      if(client_fd < 0) perror("Client connection error");

      for(int i = 2; i < MAX_CLIENTS + 2; i++) {
        if(pfds[i].fd == -1) {
          pfds[i].fd = client_fd;
          nfds++;

          char name[128];
          int n = read(client_fd, name, sizeof(name));
          name[n] = '\0';
 
          write(STDOUT_FILENO, ">", 1);
          write(STDOUT_FILENO, "Connection found", 18);
          write(STDOUT_FILENO, name, n);
          write(STDOUT_FILENO, "\n", 1);

          map_insert(Users, strdup(name), client_fd);
          break;
        }
      }
    }
    for(int i = 2; i < nfds; i++) {
      if(pfds[i].fd == -1) continue;

      if(pfds[i].revents & POLLIN) {
        char buffer[256];
        int file_fd = open("test", O_APPEND | O_CREAT, 0644);
        size_t n = read(pfds[i].fd, buffer, sizeof(buffer));
        write(file_fd, buffer, n);
        
        close(file_fd);
      }
    }
  }


  return 0;
}
