#include "server.h"
#include <iostream>
#include <algorithm>
#include "utilities/json.hpp"


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

void com_out::Server::registerRequestListener(std::shared_ptr<IRequestListener> listener) {
  _requestListeners.push_back(listener);
}

void com_out::Server::deleteRequestListener(std::shared_ptr<IRequestListener> listener) {
  _requestListeners.erase(std::remove(_requestListeners.begin(), _requestListeners.end(), listener), _requestListeners.end());
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
  for (;;) {
    // get a request
    std::string request = getRequest(client);
    // break if client is done or an error occurred
    if (request.empty()) {
      break;
    }

    auto jsonRequest = nlohmann::json::parse(request);
    nlohmann::json jsonResponse = {
      {"type", "server.callback"},
      {"cbIndex", jsonRequest.value("cbIndex", -1)}, // in case cbIndex does not exist, default value of -1 is used
      {"data", {}},
    };

    std::cout << request << std::endl;

    // pass to every request listener and collect responses
    for(auto const& listener: _requestListeners) {
      void(listener->handleRequest(jsonRequest["type"], jsonRequest, jsonResponse["data"]));
    }

    std::string res = jsonResponse.dump() + "\n";
    if (!sendToClient(client, res)) {
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

bool com_out::Server::sendToClient(int client, const std::string msg) const {
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

bool com_out::Server::broadcast(const std::string msg) const {
  for(int client: _clients) {
    sendToClient(client, msg);
  }
}
