#include "tcp_server.h"
#include <iostream>


com_out::TcpServer::TcpServer(output::Storage& outputStorage) : Server(outputStorage) {}

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

  const int sockOpt = 1; 
  if (setsockopt(_server, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &sockOpt, sizeof(sockOpt))) {
    throw std::runtime_error("Error on setting socket options");
  }

  // This option is used to reduce latency by forcing every message to start with its own tcp package
  const int tcpOpt = 1;
  if (setsockopt(_server, SOL_TCP, TCP_NODELAY, &tcpOpt, sizeof(tcpOpt))) {
    throw std::runtime_error("Error on setting tcp options");
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
