#ifndef PROTOCOL_PAYLOAD_H
#define PROTOCOL_PAYLOAD_H

#include "protocol_types.h"
#include "protocol_codec.h"
#include "string.h"
#include "protocol_frame.h"

// Login payload rules: The first 16 bits is length of the name, and length of the password after the name
int decode_payload_login(Buff *b, MsgLogin *p);
int encode_payload_login(Buff *b, MsgLogin *p);

// OK 
int send_ok(int fd);

// Error 
// The 16 bits after first byte is length of next message
int decode_payload_error(Buff *b, MsgError *e);
int encode_payload_error(Buff *b, MsgError *e);
int send_error(int fd, ErrorTypes type);

#endif
