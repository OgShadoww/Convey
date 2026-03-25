CC = clang
TARGET = server

CFLAGS = -Wall -Wextra -g -O0
LDLIBS = $(shell pkg-config --libs openssl)
CFLAGS += $(shell pkg-config --cflags openssl)

SOURCE = server.c \
	../protocol/src/protocol_codec.c \
	../protocol/src/protocol_frame.c \
	../protocol/src/protocol_errors.c \
	../protocol/src/protocol_payloads.c \
	../protocol/src/protocol_transport.c

OBJ = $(SOURCE:.c=.o)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(LDLIBS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJ)
