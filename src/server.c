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
  int i, port;
  char char_port[BUFSIZ], char_dir[BUFSIZ];

  // set default port and directory if nothing is passed in
  port = 8080;
  strncpy(char_port, "8080", sizeof(char_port));
  strncpy(char_dir, "html", sizeof(char_dir));

  if (argc > 1) {
    for (i = 1; i < argc; i++) {
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
  start_server(char_dir, char_port);
}
