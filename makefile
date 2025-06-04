CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pthread
BIN_DIR = bin

SRC_COMMON = common.c connect_functions.c
SRC_SERVER = server.c client_handler.c $(SRC_COMMON)
SRC_CLIENT = client.c $(SRC_COMMON)

OBJS_SERVER = $(SRC_SERVER:.c=.o)
OBJS_CLIENT = $(SRC_CLIENT:.c=.o)

all: $(BIN_DIR) $(BIN_DIR)/server $(BIN_DIR)/client

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/server: $(SRC_SERVER)
	$(CC) $(CFLAGS) $(SRC_SERVER) -o $(BIN_DIR)/server -lm
$(BIN_DIR)/client: $(SRC_CLIENT)
	$(CC) $(CFLAGS) $(SRC_CLIENT) -o $(BIN_DIR)/client

clean:
	rm -f *.o
	rm -rf $(BIN_DIR)/*

.PHONY: all clean
