#include "unix_server.h"
#include <iostream>


const char* com_out::UnixServer::_socketName = "/tmp/unix-socket";

com_out::UnixServer::UnixServer() {
  // Remove unix sockets file (_socketName)
  // setup handler for Control-C so we can properly unlink the UNIX socket when that occurs
  struct sigaction sigIntHandler;
  sigIntHandler.sa_handler = interrupt;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;
  sigaction(SIGINT, &sigIntHandler, NULL);
}

void com_out::UnixServer::create() {
  struct sockaddr_un serverAddr;

  // setup socket address structure
  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sun_family = AF_UNIX;
  strncpy(serverAddr.sun_path, _socketName, sizeof(serverAddr.sun_path) - 1);

  // create socket
  _server = socket(PF_UNIX, SOCK_STREAM, 0);
  if(!_server) {
    throw std::runtime_error("Error on socket creation");
  }

  // Just to make sure the _socketName (e.g. if path exists) is not already linked
  unlink(_socketName);
  // call bind to associate the socket with the UNIX file system
  if(bind(_server, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
    throw std::runtime_error("Error on socket binding");
  }

  // convert the socket to listen for incoming connections
  if(listen(_server, SOMAXCONN) < 0) {
    throw std::runtime_error("Error on socket listen");
  }
}

void com_out::UnixServer::closeSocket() {
  unlink(_socketName);
}

void com_out::UnixServer::interrupt(int) {
  unlink(_socketName);
}
