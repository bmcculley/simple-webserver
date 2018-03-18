#ifndef LIBSERVER_H_
#define LIBSERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSIZE 8096
#define ERROR 42
#define SORRY 43
#define LOG   44

#ifndef SIGCLD
#   define SIGCLD SIGCHLD
#endif

typedef struct {
  char *ext;
  char *filetype;
} f_extensions;

void server_log(int type, char *s1, char *s2, int num);
void web(int fd, int hit);

#endif