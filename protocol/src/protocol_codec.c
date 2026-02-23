#include "../include/protocol_codec.h"

// Write Operations
int buff_write_u8(Buff *b, uint8_t v) {
  if(b->pos + 1 > b->len) return -1;
  b->data[b->pos++] = v;

  return 0;
}

int buff_write_u16(Buff *b, uint16_t v) {
  if(b->pos + 2 > b->len) return -1;

  b->data[b->pos+0] = (uint8_t)((v >> 8) & 0xFF);
  b->data[b->pos+1] = (uint8_t)((v >> 0) & 0xFF);

  b->pos += 2;
  return 0;
}

int buff_write_u32(Buff *b, uint32_t v) {
  if(b->pos+4 > b->len) return -1;

  b->data[b->pos+0] = (uint8_t)((v>>24) & 0xFF);
  b->data[b->pos+1] = (uint8_t)((v>>16) & 0xFF);
  b->data[b->pos+2] = (uint8_t)((v>>8) & 0xFF);
  b->data[b->pos+3] = (uint8_t)((v>>0) & 0xFF);

  b->pos += 4;
  return 0;
}

// Read Operations
int buff_read_u8(Buff *b, uint8_t *out) {
  if(b->pos + 1 > b->len) return -1;
  *out = b->data[b->pos++];

  return 0;
}

int buff_read_u16(Buff *b, uint16_t *out) {
  if(b->pos + 2 > b->len) return -1;

  uint16_t p = 0;
  p |= (uint16_t)b->data[b->pos+0] << 8;
  p |= (uint16_t)b->data[b->pos+1] << 0;

  *out = p;
  b->pos += 2;

  return 0;
}

int buff_read_u32(Buff *b, uint32_t *out) {
  if(b->pos + 4 > b->len) return -1;

  uint32_t p = 0;
  p |= (uint32_t)b->data[b->pos+0] << 24;
  p |= (uint32_t)b->data[b->pos+1] << 16;
  p |= (uint32_t)b->data[b->pos+2] << 8;
  p |= (uint32_t)b->data[b->pos+3] << 0;

  *out = p;
  b->pos += 4;

  return 0;
}
