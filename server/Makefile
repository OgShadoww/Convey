CC = clang
TARGET = server
FLAGS = -Wall -g -o0 -Wextra -o
SOURCE = server.c ../protocol/src/protocol_codec.c ../protocol/src/protocol_frame.c ../protocol/src/protocol_errors.c ../protocol/src/protocol_payloads.c ../protocol/src/protocol_transport.c

$(TARGET): Makefile $(SOURCE)
	$(CC) $(SOURCE) $(FLAGS) $(TARGET)
