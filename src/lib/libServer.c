#include "libServer.h"

f_extensions extensions[] = {
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
  int fd;
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
    (void)strncpy(buffer, "GET /index.html", sizeof(buffer));
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

int start_server(int port, char * char_dir, char * char_port)
{
  int i, pid, listenfd, socketfd, hit;
  socklen_t length;
  static struct sockaddr_in cli_addr; 
  static struct sockaddr_in serv_addr;

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