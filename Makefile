
all: server

server:	server.c
	$(CC) -Wall -std=c99 server.c -o server

clean:
	rm -f server
