CC = clang
TARGET = client

OPENSSL_PREFIX := $(shell brew --prefix openssl@3)

CFLAGS = -Wall -Wextra -g -O0 -I$(OPENSSL_PREFIX)/include
LDFLAGS = -L$(OPENSSL_PREFIX)/lib -lssl -lcrypto

SOURCE = client.c \
	../protocol/src/protocol_codec.c \
	../protocol/src/protocol_frame.c \
	../protocol/src/protocol_errors.c \
	../protocol/src/protocol_payloads.c \
	../protocol/src/protocol_transport.c

$(TARGET): Makefile $(SOURCE)
	$(CC) $(CFLAGS) $(SOURCE) $(LDFLAGS) -o $(TARGET)

clean:
	rm -f $(TARGET)

