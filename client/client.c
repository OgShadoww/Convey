#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>

const char *welcome_msg[] = {
    "Welcome to the Convey server!\n",
    "Please enter your username and press Enter: ",
    "Now please write the file name: "
};

int main() {
  char *name;
  write(STDOUT_FILENO, welcome_msg[0], strlen(welcome_msg[0]));
  write(STDOUT_FILENO, welcome_msg[1], strlen(welcome_msg[1]));

  int n = read(STDIN_FILENO, name, sizeof(name));
  write(STDOUT_FILENO, "\n", 1);
  write(STDOUT_FILENO, welcome_msg[2], strlen(welcome_msg[2]));

  char *file_n;
  int file_size = read(STDIN_FILENO, file_n, sizeof(file_n));

  int file_fd = open(file_n, O_RDONLY);
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(8080);
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

  connect(socket_fd, (struct sockaddr*)&addr, sizeof(addr)); 
  write(socket_fd, name, n);

  char buff[128];
  int message_size = read(STDIN_FILENO, buff, sizeof(buff));
  write(socket_fd, buff, message_size);
  
  char file_buffer[1024];
  while((n = read(file_fd, file_buffer, sizeof(file_buffer))) > 0) {
    write(socket_fd, file_buffer, n);
  }

  close(socket_fd);

  return 0;
}
