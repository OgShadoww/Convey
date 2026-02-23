#include "../include/protocol_payloads.h"

// Login decode/encode
int decode_payload_login(Buff *b, MsgLogin *p) {
  uint16_t ul = 0, pl = 0;
  if(buff_read_u16(b, &ul) == -1) return -1;

  if(ul >= sizeof(p->username)+1) return -1;

  for(int i = 0; i < (int)ul; i++) {
    uint8_t ch;
    if(buff_read_u8(b, &ch) == -1) return -1;

    p->username[i] = (char)ch;
  }
  p->username[ul] = '\0';

  if(buff_read_u16(b, &pl) == -1) return -1;

  if(pl >= sizeof(p->password)+1) return -1;

  for(int i = 0; i < pl; i++) {
    uint8_t ch;
    if(buff_read_u8(b, &ch) == -1) return -1;

    p->password[i] = (char)ch;
  }
  p->password[pl] = '\0';

  return 0;
}

int encode_payload_login(Buff *b, MsgLogin *p) {
  uint16_t ul = strlen(p->username);
  uint16_t pl = strlen(p->password);

  if (b->pos + (size_t)(2 + ul + 2 + pl) > b->len) return -1;

  if(buff_write_u16(b, ul) == -1) return -1;
  for(int i = 0; i < ul; i++) {
    if(buff_write_u8(b, (uint8_t)p->username[i]) == -1) return -1;
  }
  if(buff_write_u16(b, pl) == -1) return -1;
  for(int i = 0; i < pl; i++) {
    if(buff_write_u8(b, (uint8_t)p->password[i]) == -1) return -1;
  }

  return 0;
}

// OK 
int send_ok(int fd) {
  ConveyFrame f = { 
    .header = {
      .magic = CONVEY_MAGIC,
      .version = CONVEY_VERSION,
      .type = MSG_OK,
      .payload_len = 0
    },
    .payload = { 
      .data = NULL,
      .pos = 0,
      .len = 0
    }
  };

  write_frame(fd, &f);

  return 0;
}

// Error
int decode_payload_error(Buff *b, MsgError *e) {
  uint8_t type = 0;
  if(buff_read_u8(b, &type) == -1) return -1;
  e->error_type = type;

  return 0;
}

int encode_payload_error(Buff *b, MsgError *e) {
  if(buff_write_u8(b, e->error_type) == -1) return -1;

  return 0;
}

int send_error(int fd, ErrorTypes type) {
  uint32_t payload_len = sizeof(uint8_t);

  MsgError e = { 
    .error_type = type,
  };
  Buff p; 
  if(encode_payload_error(&p, &e) == -1) return -1;

  ConveyFrame f = {
    .header = {
      .magic = CONVEY_MAGIC,
      .version = CONVEY_VERSION,
      .type = MSG_ERROR,
      .payload_len = payload_len
    },
    .payload = p
  };

  return 0;
}

