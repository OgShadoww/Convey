#ifndef PROTOCOL_TYPES_H
#define PROTOCOL_TYPES_H

#include <cstdint>
#include <stddef.h>
#include <stdint.h>

typedef struct {
  uint8_t *data;
  size_t len;
  size_t pos;
} Buff;

typedef enum {
  MSG_AUTH_REGISTER = 1,
  MSG_AUTH_LOGIN = 2,
  MSG_GET = 3,
  MSG_PUT_INIT = 4,
  MSG_PUT_CHUNK = 5,
  MSG_PUT_FINISH = 6,
  MSG_ERROR = 80,
  MSG_OK = 88
} ConveyMsgType;

typedef enum {
  ERR_WRONG_TYPE,
  ERR_PERMISSION_DENIED,
  ERR_CONNECTION_FAILLED
} ErrorTypes;

#define CONVEY_MAGIC 0x43565931u
#define CONVEY_HEADER_LEN 10
#define CONVEY_MAX_PAYLOAD (8u * 1024u * 1024u)
#define CONVEY_VERSION 1

typedef struct {
  uint32_t magic;
  uint8_t version;
  uint8_t type;
  uint32_t payload_len;
  uint8_t token[];
} ConveyHeader;

typedef struct {
  ConveyHeader header;
  Buff payload;
} ConveyFrame;

// In the binary first 16 bits contain length
typedef struct {
  char username[128];
  char password[128];
} MsgLogin;

typedef struct {
  char username[128];
  char password[128];
  char confirmed_password[128];
} MsgRegister;

typedef struct {
  uint8_t error_type;
} MsgError;

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

#endif
