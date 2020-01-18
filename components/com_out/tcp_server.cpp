#include "tcp_server.h"
#include <iostream>


com_out::TcpServer::TcpServer(const output::Storage& outputStorage) : Server(outputStorage) {
  // setup handler for Control-C so we can properly unlink the UNIX socket when that occurs
  // struct sigaction sigIntHandler;
  // sigIntHandler.sa_handler = interrupt;
  // sigemptyset(&sigIntHandler.sa_mask);
  // sigIntHandler.sa_flags = 0;
  // sigaction(SIGINT, &sigIntHandler, NULL);
}

void com_out::TcpServer::create() {
  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(8999);

  // create socket
  _server = socket(AF_INET, SOCK_STREAM, 0);
  if(!_server) {
    throw std::runtime_error("Error on socket creation");
  }

  const int opt = 1; 
  if (setsockopt(_server, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
    throw std::runtime_error("Error on setting socket options");
  }

  // call bind to associate the socket with the UNIX file system
  if(bind(_server, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
    throw std::runtime_error("Error on socket binding");
  }

  // convert the socket to listen for incoming connections
  if(listen(_server, SOMAXCONN) < 0) {
    throw std::runtime_error("Error on socket listen");
  }
}

void com_out::TcpServer::closeSocket() {
  close(_server);
}

// void com_out::TcpServer::interrupt(int) {
//   close(_server);
// }
