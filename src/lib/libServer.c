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