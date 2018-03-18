/**
 * project:   miniweb
 * author:    Oscar Sanchez (oms1005@gmail.com)
 * HTTP Server
 * WORKS ON BROWSERS TOO!
 * Inspired by IBM's nweb
 */

#include "lib/libServer.h"

void help_msg(char * progname) {
    printf("This is a simple web server to serve static files.\n" \
    "\n" \
    "Usage: %s -p 8080 -d html\n" \
    "\n" \
    "    -h, --help              Display this help message.\n" \
    "    -p, --port VALUE        The port for the server to listen on. \n" \
    "                            Defaults to 8080.\n" \
    "    -d, --directory VALUE   The directory to serve files from.\n" \
    "                            Defaults to html.\n" \
    "\n" \
    "The source code for this can be found on github: \n" \
    "https://github.com/bmcculley/simple-webserver\n",
    progname );
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
  strncpy(char_port, "8080", sizeof(char_port));
  strncpy(char_dir, "html", sizeof(char_dir));

  if (argc > 1) {
    for (int i = 1; i < argc; i++) {
      if ( strcmp(argv[i], "--port") == 0 || strcmp(argv[i], "-p") == 0) {
        i++;
        port = atoi(argv[i]);
        strncpy(char_port, argv[i], sizeof(char_port));
        if(port < 0 || port > 60000) {
          server_log(ERROR, "Invalid port number try [1,60000]", char_port, 0);
        }
      }
      if ( strcmp(argv[i], "--directory") == 0 || strcmp(argv[i], "-d") == 0) {
        i++;
        strncpy(char_dir, argv[i], sizeof(char_dir));
      }
      if ( strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
        help_msg(argv[0]);
        exit(0);
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
