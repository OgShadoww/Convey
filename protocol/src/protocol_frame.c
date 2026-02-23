#include "../include/protocol_frame.h"

// ===============================
// FRAME HEADER OPERATIONS
// ===============================

int encode_header(Buff *b, ConveyHeader *h) {
  if(buff_write_u32(b, h->magic) == -1) return -1;
  if(buff_write_u8(b, h->version) == -1) return -1;
  if(buff_write_u8(b, h->type) == -1) return -1;
  if(buff_write_u32(b, h->payload_len) == -1) return -1;

  return 0;
}

int decode_header(Buff *b, ConveyHeader *h) {
  if(buff_read_u32(b, &h->magic) == -1)  return -1;
  if(buff_read_u8(b, &h->version) == -1) return -1;
  if(buff_read_u8(b, &h->type) == -1) return -1;
  if(buff_read_u32(b, &h->payload_len) == -1) return -1;

  return 0;
}

// ===============================
// FRAME OPERATIONS READ / WRITE
// ===============================

int read_frame(int fd, ConveyFrame *f) {
  // Read header
  uint8_t header_bytes[CONVEY_HEADER_LEN];
  if(read_exact(fd, header_bytes, CONVEY_HEADER_LEN) == -1) return -1;

  Buff b = {.data = header_bytes, .len = CONVEY_HEADER_LEN, .pos = 0};
  if(decode_header(&b, &f->header) == -1) return -1;

  if(f->header.magic != CONVEY_MAGIC || f->header.payload_len > CONVEY_MAX_PAYLOAD || f->header.version != CONVEY_VERSION) return -1;

  if(f->header.payload_len > 0) {
    f->payload.data = malloc(f->header.payload_len);
    if(f->payload.data == NULL) return -1;
  }
  else {
    f->payload.data = NULL;
  }
  f->payload.len = f->header.payload_len;
  f->payload.pos = 0;

  if(read_exact(fd, f->payload.data, f->header.payload_len) == -1) {
    if(f->payload.len > 0) {
      free(f->payload.data);
    }
    return -1;
  }

  return 0;
}

int write_frame(int fd, ConveyFrame *f) {
  // Write header
  uint8_t header_bytes[CONVEY_HEADER_LEN];
  Buff hb = {.data = header_bytes, .len = CONVEY_HEADER_LEN, .pos = 0};

  f->header.payload_len = (uint32_t)f->payload.len;

  if(encode_header(&hb, &f->header) == -1) return -1;
  if(write_all(fd, header_bytes, CONVEY_HEADER_LEN) == -1) return -1;

  // Write payload
  if(f->payload.len > 0) {
    if(f->payload.data == NULL) return -1;
    if(write_all(fd, f->payload.data, f->payload.len) == -1) return -1;
  }

  return 0;
}

void free_frame(ConveyFrame *f) {
  free(f->payload.data);
  f->payload.data = NULL;
  f->payload.len = 0;
  f->payload.pos = 0;
}
