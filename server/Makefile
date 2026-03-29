CC := clang
TARGET := server
PKG_CONFIG := pkg-config

CFLAGS := -Wall -Wextra -g -O0 \
  -Iinclude \
  -I../protocol/include \
  $(shell $(PKG_CONFIG) --cflags openssl sqlite3)

LDLIBS := \
  $(shell $(PKG_CONFIG) --libs openssl sqlite3)

BUILD_DIR := build

SERVER_SRC := \
  src/main.c \
  src/server.c \
  src/server_client.c \
  src/server_handlers.c \
  src/server_net.c \
  src/db.c

PROTOCOL_SRC := \
  ../protocol/src/protocol_codec.c \
  ../protocol/src/protocol_frame.c \
  ../protocol/src/protocol_errors.c \
  ../protocol/src/protocol_payloads.c \
  ../protocol/src/protocol_transport.c

SERVER_OBJ := $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SERVER_SRC))
PROTOCOL_OBJ := $(patsubst ../protocol/src/%.c,$(BUILD_DIR)/protocol/%.o,$(PROTOCOL_SRC))
OBJ := $(SERVER_OBJ) $(PROTOCOL_OBJ)

.PHONY: all clean
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(LDLIBS) -o $@

$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/protocol/%.o: ../protocol/src/%.c
	@mkdir -p $(BUILD_DIR)/protocol
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
