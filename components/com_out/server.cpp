#include "server.h"
#include <iostream>


com_out::Server::Server() {
  _buflen = 1024;
  _buf = new char[_buflen+1];
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
    handle(client);
  }

  closeSocket();
}

void com_out::Server::handle(int client) {
  // loop to handle all requests
  for(;;) {
    // get a request
    std::string request = getRequest(client);
    // break if client is done or an error occurred
    if(request.empty()) {
      break;
    }
    // send response
    // TODO: actually do some routing here
    if(!sendResponse(client, request)) {
      break;
    }
  }
  close(client);
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

bool com_out::Server::sendResponse(int client, std::string response) {
  // prepare to send response
  const char* ptr = response.c_str();
  int nleft = response.length();
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
        throw std::runtime_error("Error on writing");
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
