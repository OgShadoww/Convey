#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#include "../protocol/include/protocol.h"

static int read_line(char *buf, size_t size) {
  if (!buf || size == 0) return -1;
  if (fgets(buf, size, stdin) == NULL) return -1;
  buf[strcspn(buf, "\n")] = '\0';
  return 0;
}

int main(void) {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) { perror("socket"); return 1; }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(8080);

  if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0) {
    perror("inet_pton");
    close(fd);
    return 1;
  }

  if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    perror("connect");
    close(fd);
    return 1;
  }

  printf("Type: login / exit\n");

  char cmd[128];
  while (1) {
    printf("> ");
    fflush(stdout);
    if (read_line(cmd, sizeof(cmd)) < 0) break;

    if (strcmp(cmd, "exit") == 0) break;

    if (strcmp(cmd, "login") == 0) {
      char name[128], password[128];

      printf("Username: ");
      fflush(stdout);
      if (read_line(name, sizeof(name)) < 0) break;

      printf("Password: ");
      fflush(stdout);
      if (read_line(password, sizeof(password)) < 0) break;

      MsgLogin p = {0};
      strncpy(p.username, name, sizeof(p.username) - 1);
      strncpy(p.password, password, sizeof(p.password) - 1);

      // Encode payload first (so header.payload_len is correct).
      size_t ul = strlen(p.username);
      size_t pl = strlen(p.password);

      if (ul > 65535 || pl > 65535) {
        fprintf(stderr, "username/password too long\n");
        continue;
      }

      size_t payload_cap = 2 + ul + 2 + pl;
      uint8_t *payload = malloc(payload_cap);
      if (!payload) { perror("malloc payload"); break; }

      Buff pb = { .data = payload, .len = payload_cap, .pos = 0 };
      if (encode_payload_login(&pb, &p) < 0) {
        fprintf(stderr, "encode_payload_login failed\n");
        free(payload);
        continue;
      }

      // Build header bytes.
      uint8_t header_bytes[CONVEY_HEADER_LEN];
      Buff hb = { .data = header_bytes, .len = CONVEY_HEADER_LEN, .pos = 0 };

      ConveyHeader header = {0};
      header.magic = CONVEY_MAGIC;
      header.version = CONVEY_VERSION;
      header.type = MSG_AUTH_LOGIN;
      header.payload_len = (uint32_t)pb.pos;
      header.token = 0; // not authenticated yet

      if (encode_header(&hb, &header) < 0) {
        fprintf(stderr, "encode_header failed\n");
        free(payload);
        continue;
      }

      // Send header then payload.
      if (write_all(fd, header_bytes, CONVEY_HEADER_LEN) < 0) {
        free(payload);
        break;
      }
      if (write_all(fd, payload, pb.pos) < 0) {
        free(payload);
        break;
      }

      printf("Login frame sent (payload=%zu bytes).\n", pb.pos);
      free(payload);
    } else {
      printf("Unknown command.\n");
    }
  }

  close(fd);
  return 0;
}
