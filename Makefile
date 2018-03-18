MKDIR_P = mkdir -p
BUILD_DIR = build
SRC_DIR = src

all: server

server:
	test -d $(BUILD_DIR) || $(MKDIR_P) $(BUILD_DIR)
	$(CC) -c -o $(BUILD_DIR)/libServer.o $(SRC_DIR)/lib/libServer.c
	ar rcs $(BUILD_DIR)/libServer.a $(BUILD_DIR)/libServer.o
	$(CC) -c $(SRC_DIR)/server.c -o $(BUILD_DIR)/server.o
	$(CC) -Wall -std=c99 $(BUILD_DIR)/server.o $(BUILD_DIR)/libServer.a -o $(BUILD_DIR)/server
	rm $(BUILD_DIR)/libServer.* $(BUILD_DIR)/server.o

clean:
	rm -rf $(BUILD_DIR)
