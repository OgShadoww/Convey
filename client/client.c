#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "../protocol/include/protocol.h"

int main() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(8080);

  if(inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0) return -1;

  if(connect(fd, &addr, sizeof(addr)) < 0) return -1;
  
  write(STDOUT_FILENO, "Hello \n Interface: \n 1.Login \n 2.Register 3.", strlen("Hello \n Interface: \n 1.Login \n 2.Register 3."));

  char buff[128];
  size_t n;
  while((n =read(STDOUT_FILENO, buff, sizeof(buff)) != -1)) {
    if(strcmp(buff, "login") == 0) {
      
    }
  }


  return 0;
}
