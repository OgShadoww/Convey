#include "../include/server.h"

int main() {
  Server server = {0};
  if(server_init(&server) == -1) {
    return -1;
  }
  
  server_run(&server);
}
