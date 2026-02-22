#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../protocol/include/protocol.h"

#define PORT 8080

int run_server() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(8080);

  if(inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0) return -1;

  if(bind(fd, &addr, sizeof(addr)) < 0) return -1;

  if(listen(fd, 128) < 0) return -1;

  printf("Listening on 127.0.0.1:8080...\n");

  return fd;
}

void handle_request(ConveyFrame *f) {

  switch (f->header.type) {
    case MSG_AUTH_LOGIN: {
      MsgLogin l = {0};
      decode_payload_login(&f->payload, &l);
      printf("%s %s\n", l.password, l.username);
      break;
    }
  }
}

void handle_connection(int fd) {
  for(;;) {
    ConveyFrame *f = malloc(sizeof(ConveyFrame));
    read_frame(fd, f);
    handle_request(f);
    free_frame(f);
    free(f);
  }
    
}

int main() {
  int fd = run_server();

  for(;;) { 
    int cfd = accept(fd, NULL, NULL);
    if(cfd == -1) {
      printf("Error\n");
      break;
    }
    handle_connection(cfd);
  }
}
