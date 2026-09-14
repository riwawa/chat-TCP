CC = clang
CFLAGS = -Wall -Wextra -Wpedantic -Iinclude

SRC_DIR = src
BUILD_DIR = build

COMMON_SRC = \
	$(SRC_DIR)/network.c \
	$(SRC_DIR)/framing.c \
	$(SRC_DIR)/lexer.c \
	$(SRC_DIR)/token_debug.c

SERVER_SRC = $(SRC_DIR)/server.c $(COMMON_SRC)
CLIENT_SRC = $(SRC_DIR)/client.c $(COMMON_SRC)

SERVER_BIN = $(BUILD_DIR)/server
CLIENT_BIN = $(BUILD_DIR)/client

all: $(SERVER_BIN) $(CLIENT_BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(SERVER_BIN): $(SERVER_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SERVER_SRC) -o $(SERVER_BIN)

$(CLIENT_BIN): $(CLIENT_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CLIENT_SRC) -o $(CLIENT_BIN)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean