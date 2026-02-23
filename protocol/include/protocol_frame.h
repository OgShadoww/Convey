#ifndef PROTOCOL_FRAME_H
#define PROTOCOL_FRAME_H

#include "protocol_types.h"
#include "protocol_codec.h"
#include "protocol_transport.h"
#include <stdlib.h>

// Framing
int read_frame(int fd, ConveyFrame *f);
int write_frame(int fd, ConveyFrame *f);
void free_frame(ConveyFrame *f);

// Encode / Decode header bytes
int decode_header(Buff *b, ConveyHeader *h);
int encode_header(Buff *b, ConveyHeader *h);

#endif
