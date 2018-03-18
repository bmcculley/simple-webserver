/**
 * project:   miniweb
 * author:    Oscar Sanchez (oms1005@gmail.com)
 * HTTP Server
 * WORKS ON BROWSERS TOO!
 * Inspired by IBM's nweb
 */

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

struct {
  char *ext;
  char *filetype;
} extensions [] = {
  {"gif",  "image/gif"    },  
  {"jpg",  "image/jpeg"   }, 
  {"jpeg", "image/jpeg"   },
  {"png",  "image/png"    }, 
  {"ico",  "image/x-icon" }, 
  {"zip",  "image/zip"    },  
  {"gz",   "image/gz"     },  
  {"tar",  "image/tar"    },  
  {"htm",  "text/html"    },  
  {"html", "text/html"    },  
  {"php",  "image/php"    },  
  {"cgi",  "text/cgi"     },  
  {"asp",  "text/asp"     },  
  {"jsp",  "image/jsp"    },  
  {"xml",  "text/xml"     },  
  {"js",   "text/js"      },
  {"css",  "test/css"     }, 
  {0,0} 
};

void server_log(int type, char *s1, char *s2, int num)
{
  int fd ;
  char logbuffer[BUFSIZE*2];

  switch (type) {
    case ERROR: 
      (void)snprintf(logbuffer, sizeof(logbuffer), "ERROR: %s:%s Errno=%d exiting pid=%d", s1, s2, errno, getpid()); 
      break;
    case SORRY: 
      (void)snprintf(logbuffer, sizeof(logbuffer), "<html><body><h1>Web Server Sorry: %s %s</h1></body></html>\r\n", s1, s2);
      (void)write(num,logbuffer,strlen(logbuffer));
      (void)snprintf(logbuffer, sizeof(logbuffer), "SORRY: %s:%s",s1, s2); 
      break;
    case LOG: 
      (void)snprintf(logbuffer, sizeof(logbuffer), "INFO: %s:%s:%d",s1, s2,num); 
      break;
  }  
  
  if((fd = open("server.log", O_CREAT| O_WRONLY | O_APPEND,0644)) >= 0) {
    (void)write(fd,logbuffer,strlen(logbuffer)); 
    (void)write(fd,"\n",1);    
    (void)close(fd);
  }
  if(type == ERROR || type == SORRY) exit(3);
}

void web(int fd, int hit)
{
  int j, file_fd, buflen, len;
  long i, ret;
  char * fstr;
  static char buffer[BUFSIZE+1];

  ret = read(fd, buffer, BUFSIZE); 
  if(ret == 0 || ret == -1) {
    server_log(SORRY,"failed to read browser request","",fd);
  }
  if(ret > 0 && ret < BUFSIZE) {
    buffer[ret]=0;  
  }
  else buffer[0]=0;

  for(i=0; i < ret; i++) {
    if(buffer[i] == '\r' || buffer[i] == '\n') {
      buffer[i]='*';
    }
  }
  server_log(LOG, "request", buffer, hit);

  if( strncmp(buffer,"GET ",4) && strncmp(buffer,"get ",4) ) {
    server_log(SORRY, "Only simple GET operation supported", buffer, fd);
  }

  for(i=4;i<BUFSIZE;i++) { 
    if(buffer[i] == ' ') { 
      buffer[i] = 0;
      break;
    }
  }

  for(j=0; j < i - 1; j++) {
    if(buffer[j] == '.' && buffer[j+1] == '.') {
      server_log(SORRY, "Parent directory (..) path names not supported", buffer, fd);
    }
  }

  if( !strncmp(&buffer[0], "GET /\0",6) || !strncmp(&buffer[0], "get /\0",6) ) {
    (void)strlcpy(buffer, "GET /index.html", sizeof(buffer));
  }

  buflen = strlen(buffer);
  fstr = (char *)0;

  for(i=0; extensions[i].ext != 0; i++) {
    len = strlen(extensions[i].ext);
    if( !strncmp(&buffer[buflen-len], extensions[i].ext, len)) {
      fstr = extensions[i].filetype;
      break;
    }
  }
  if(fstr == 0) server_log(SORRY,"file extension type not supported",buffer,fd);

  if(( file_fd = open(&buffer[5],O_RDONLY)) == -1) 
    server_log(SORRY, "failed to open file",&buffer[5],fd);

  server_log(LOG, "SEND", &buffer[5], hit);

  (void)snprintf(buffer, sizeof(buffer), "HTTP/1.0 200 OK\r\nContent-Type: %s\r\n\r\n", fstr);
  (void)write(fd, buffer, strlen(buffer));

  while (  (ret = read(file_fd, buffer, BUFSIZE)) > 0 ) {
    (void)write(fd,buffer,ret);
  }
#ifdef LINUX
  sleep(1);
#endif
  exit(1);
}


int main(int argc, char **argv)
{
  int i, port, pid, listenfd, socketfd, hit;
  socklen_t length;
  static struct sockaddr_in cli_addr; 
  static struct sockaddr_in serv_addr;
  char char_port[BUFSIZ], char_dir[BUFSIZ];

  // set default port and directory if nothing is passed in
  port = 8080;
  strlcpy(char_port, "8080", sizeof(char_port));
  strlcpy(char_dir, "html", sizeof(char_dir));

  if (argc > 1) {
    for (int i = 1; i < argc; i++) {
      if ( strcmp(argv[i], "--port") == 0 || strcmp(argv[i], "-p") == 0) {
        i++;
        port = atoi(argv[i]);
        strlcpy(char_port, argv[i], sizeof(char_port));
        if(port < 0 || port > 60000) {
          server_log(ERROR, "Invalid port number try [1,60000]", char_port, 0);
        }
      }
      if ( strcmp(argv[i], "--directory") == 0 || strcmp(argv[i], "-d") == 0) {
        i++;
        strlcpy(char_dir, argv[i], sizeof(char_dir));
      }
    }
  }

  if( !strncmp(char_dir, "/", 2 ) || !strncmp(char_dir, "/etc", 5 ) ||
      !strncmp(char_dir, "/bin", 5 ) || !strncmp(char_dir, "/lib", 5 ) ||
      !strncmp(char_dir, "/tmp", 5 ) || !strncmp(char_dir, "/usr", 5 ) ||
      !strncmp(char_dir, "/dev", 5 ) || !strncmp(char_dir, "/sbin", 6 ) ) 
  {
    (void)printf("ERROR: Bad top directory %s, see server -?\n", char_dir);
    exit(3);
  }
  if(chdir(char_dir) == -1) { 
    (void)printf("ERROR: Can't Change to directory %s\n", char_dir);
    exit(4);
  }

  if(fork() != 0) {
    return 0; 
  }
  
  (void)signal(SIGCLD, SIG_IGN); 
  (void)signal(SIGHUP, SIG_IGN);

  for(i=0; i<32; i++) {
    (void)close(i);  
  }
  //(void)setpgrp();  

  server_log(LOG, "http server starting", char_port, getpid());

  if((listenfd = socket(AF_INET, SOCK_STREAM,0)) <0){
    server_log(ERROR, "system call", "socket", 0);
  }
  
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(port);
  
  if(bind(listenfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
    server_log(ERROR, "system call", "bind", 0);
  }
  
  if( listen(listenfd,64) < 0) {
    server_log(ERROR, "system call", "listen", 0);
  }

  for(hit=1; ;hit++) {
    length = sizeof(cli_addr);
    if((socketfd = accept(listenfd, (struct sockaddr *)&cli_addr, &length)) < 0) {
      server_log(ERROR, "system call", "accept", 0);
    }

    if((pid = fork()) < 0) {
      server_log(ERROR, "system call", "fork", 0);
    }
    else {
      if(pid == 0) {
        (void)close(listenfd);
        web(socketfd,hit);
      } else {
        (void)close(socketfd);
      }
    }
  }
}
