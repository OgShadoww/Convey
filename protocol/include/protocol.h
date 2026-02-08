#include <cstdint>
#include <unistd.h>

typedef enum {
  MSG_AUTH_REGISTER,
  MSG_AUTH_LOGIN,
  MSG_GET,
  MSG_PUT_INIT,
  MSG_PUT_CHUNK,
  MSG_PUT_FINISH,
  MSG_ERROR,
  MSG_OK
} ConveyMsgType;

typedef struct {
  uint32_t magic;
  uint8_t version;
  uint8_t type;
  uint32_t payload_len;
} ConveyHeader;

typedef struct {
  ConveyHeader h;
  uint8_t *payload;
} ConveyFrame;

typedef struct {
  char username[128];
  char password[128];
} MsgLogin;

typedef struct {
  char username[128];
  char password[128];
  char confirmed_password[128];
} MsgRegister;

ssize_t conn_read(int fd, void *buff, size_t n);
ssize_t conn_write(int fd, void *buff, size_t n);

int read_header(int fd, ConveyHeader *h);
int read_payload(int fd, uint8_t *p);
int write_header(int fd, ConveyHeader *h);
int write_payload(int fd, uint8_t *p);

int read_frame(int fd, ConveyFrame *f);
int write_frame(int fd, ConveyFrame *f);

int free_frame(ConveyFrame *f);
