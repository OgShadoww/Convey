#include <stdint.h>
#include <unistd.h>

typedef struct {
  uint8_t *data;
  size_t len;
  size_t pos;
} Buff;

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

#define CONVEY_MAGIC 0x43565931u
#define CONVEY_HEADER_LEN 10
#define CONVEY_MAX_PAYLOAD (8u * 1024u * 1024u)
#define CONVEY_VERSION 1

typedef struct {
  uint32_t magic;
  uint8_t version;
  uint8_t type;
  uint32_t payload_len;
} ConveyHeader;

typedef struct {
  ConveyHeader h;
  Buff payload;
} ConveyFrame;

// In the binary first 16 bits contain length
typedef struct {
  char *username;
  char password[128];
} MsgLogin;

typedef struct {
  char username[128];
  char password[128];
  char confirmed_password[128];
} MsgRegister;

typedef struct {
  char file_name[128];
  uint64_t file_len;
} MsgPutInit;

typedef struct {
  uint64_t upload_id;
  uint64_t offset;
  uint32_t chunk_len;
  uint8_t *chunk_bytes;
} MsgPutChunk;

typedef struct {
  uint64_t upload_id;
  uint64_t file_len;
} MsgPutFinish;

// Transportation
ssize_t conn_read(int fd, void *buff, size_t n);
ssize_t conn_write(int fd, const void *buff, size_t n);

// Exact
int read_exact(int fd, void *buff, size_t len);
int write_all(int fd, const void *buff, size_t len);

// Framing
int read_frame(int fd, ConveyFrame *f);
int write_frame(int fd, ConveyFrame *f);
void free_frame(ConveyFrame *f);

// Write / Read exact bytes to buffer
int buff_write_u8(Buff *b, uint8_t v);
int buff_write_u16(Buff *b, uint16_t v);
int buff_write_u32(Buff *b, uint32_t v);
int buff_read_u8(Buff *b, uint8_t *out);
int buff_read_u16(Buff *b, uint16_t *out);
int buff_read_u32(Buff *b, uint32_t *out);

// Encode / Decode header bytes
int decode_header(Buff *b, ConveyHeader *h);
int encode_header(Buff *b, ConveyHeader *h);

// Encode / Decode payloads bytes
int decode_payload_login(Buff *b, MsgLogin *p);
int encode_payload_login(Buff *b, MsgLogin *p);
