#ifndef PROTOCOL_CODEC_H
#define PROTOCOL_CODEC_H

#include "protocol_types.h"

// Write / Read exact bytes to buffer
int buff_write_u8(Buff *b, uint8_t v);
int buff_write_u16(Buff *b, uint16_t v);
int buff_write_u32(Buff *b, uint32_t v);
int buff_read_u8(Buff *b, uint8_t *out);
int buff_read_u16(Buff *b, uint16_t *out);
int buff_read_u32(Buff *b, uint32_t *out);

#endif
