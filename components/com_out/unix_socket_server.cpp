#include <sys/socket.h>
#include <sys/un.h>
#include <iostream>
#include "unix_socket_server.h"


void com_out::startServer() {
  std::string socketPath = "hidden";
  struct sockaddr_un addr;
  char buf[100];
  int fd,cl,rc;

  if((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    throw std::runtime_error("Socket setup error...");
  }

  // Set memory to 0
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;

  if(bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    throw std::runtime_error("Socket binding error...");
  }
  if (listen(fd, 5) == -1) {
    throw std::runtime_error("Socket listen error...");
  }
}
