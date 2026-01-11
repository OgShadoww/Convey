#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
  char buffer[1024];
  int file_fd = open("main.txt", O_RDONLY);
  int new_file_fd = open("main2", O_WRONLY);

  while(read(file_fd, buffer, 1024) > 0) { 
    //write(new_file_fd, buffer, 1024);
    
  }

  return 0;
}
