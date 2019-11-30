#include "server.h"
#include <iostream>
#include <algorithm>


com_out::Server::Server() {
  _buf = new char[1024];
}

com_out::Server::~Server() {
  delete _buf;
}

void com_out::Server::run() {
  create();
  serve();
}

void com_out::Server::serve() {
  int client;
  struct sockaddr_in clientAddr;
  socklen_t clientlen = sizeof(clientAddr);

  // accept clients
  while((client = accept(_server, (struct sockaddr *)&clientAddr, &clientlen)) > 0) {
    std::cout << "Add client: " << client << std::endl;
    _clients.push_back(client);
    handle(client);
  }

  closeSocket();
}

void com_out::Server::handle(int client) {
  // loop to handle all requests
  for(;;) {
    // get a request
    std::string request = getRequest(client);
    std::cout << "Recived Request from: " << client << std::endl;
    std::cout << "Request: " << request << std::endl;
    // break if client is done or an error occurred
    if(request.empty()) {
      break;
    }
    // send response
    // TODO: actually do some routing here
    std::string msg = "{\"type\": \"dummy_response\", \"data\": \"Hello World\"}\n";
    if(!sendToClient(client, msg)) {
      break;
    }
  }
  // remove client
  _clients.erase(std::remove(_clients.begin(), _clients.end(), client), _clients.end());
  close(client);
  std::cout << "Remove client: " << client << std::endl;
}

std::string com_out::Server::getRequest(int client) {
  std::string request = "";

  // read until we get a newline
  while(request.find("\n") == std::string::npos) {
    int nread = recv(client,_buf, 1024, 0);
    if (nread < 0) {
      if (errno == EINTR)
        // the socket call was interrupted -- try again
        continue;
      else
        // an error occurred, so break out
        return "";
    } 
    else if(nread == 0) {
      // the socket is closed
      return "";
    }
    // be sure to use append in case we have binary data
    request.append(_buf, nread);
  }

  // a better server would cut off anything after the newline and
  // save it in a cache
  return request;
}

bool com_out::Server::sendToClient(int client, const std::string msg) {
  // std::cout << "Send msg " << msg << " to client: " << client << std::endl;

  // prepare to send response
  const char* ptr = msg.c_str();
  int nleft = msg.length();
  int nwritten;
  // loop to be sure it is all sent
  while(nleft) {
    if((nwritten = send(client, ptr, nleft, 0)) < 0) {
      if (errno == EINTR) {
        // the socket call was interrupted -- try again
        continue;
      } 
      else {
        // an error occurred, so break out
        throw std::runtime_error("Error on writing message to client");
        return false;
      }
    } 
    else if(nwritten == 0) {
      // the socket is closed
      return false;
    }
    nleft -= nwritten;
    ptr += nwritten;
  }
  return true;
}

bool com_out::Server::broadcast(const std::string msg) {
  for(int client: _clients) {
    sendToClient(client, msg);
  }
}
