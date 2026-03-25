CC = clang
TARGET = client

UNAME_S := $(shell uname -s)

# Default flags
CFLAGS = -Wall -Wextra -g -O0
LDFLAGS =
LDLIBS = -lssl -lcrypto

# Flags for macOS
ifeq ($(UNAME_S),Darwin)
OPENSSL_PREFIX := $(shell brew --prefix openssl@3)
CFLAGS += -I$(OPENSSL_PREFIX)/include
LDFLAGS += -L$(OPENSSL_PREFIX)/lib
endif

# Linux (APT)
ifeq ($(UNAME_S),Linux)
CFLAGS += $(shell pkg-config --cflags openssl)
LDFLAGS += $(shell pkg-config --libs-only-L openssl)
LDLIBS = $(shell pkg-config --libs openssl)
endif


SOURCE = client.c \
	../protocol/src/protocol_codec.c \
	../protocol/src/protocol_frame.c \
	../protocol/src/protocol_errors.c \
	../protocol/src/protocol_payloads.c \
	../protocol/src/protocol_transport.c

$(TARGET): Makefile $(SOURCE)
	$(CC) $(CFLAGS) $(SOURCE) $(LDFLAGS) $(LDLIBS) -o $(TARGET)

clean:
	rm -f $(TARGET)

