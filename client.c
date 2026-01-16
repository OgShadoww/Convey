#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

int main() {
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(8080);
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

  connect(socket_fd, (struct sockaddr*)&addr, sizeof(addr)); 
  write(socket_fd, "Hello", 5);
  write(socket_fd, "World", 5);

  while(1) {

  }

  close(socket_fd);

  return 0;
}
