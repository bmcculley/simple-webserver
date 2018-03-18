MKDIR_P = mkdir -p
BUILD_DIR = build
SRC_DIR = src

all: server

server:
	test -d $(BUILD_DIR) || $(MKDIR_P) $(BUILD_DIR)
	$(CC) -Wall -std=c99 $(SRC_DIR)/server.c -o $(BUILD_DIR)/server

clean:
	rm -rf $(BUILD_DIR)
