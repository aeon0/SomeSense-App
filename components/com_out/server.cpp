#include "server.h"
#include <iostream>
#include <iomanip>
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

    auto [msg, len] = createMsg(jsonResponse.dump());
    const bool success = sendToClient(client, msg, len);
    delete [] msg;
    if (!success) {
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

bool com_out::Server::sendToClient(int client, const BYTE* buf, const int len) const {
  // prepare to send response
  int nleft = len;
  int nwritten;
  // loop to be sure it is all sent
  while(nleft) {
    if((nwritten = send(client, buf, nleft, 0)) < 0) {
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
    buf += nwritten;
  }
  return true;
}

void com_out::Server::broadcast(const std::string payload) const {
  auto [msg, len] = createMsg(payload);
  for(int client: _clients) {
    sendToClient(client, msg, len);
  }
  delete [] msg;
}

std::tuple<BYTE*, int> com_out::Server::createMsg(const std::string payload) const {
  const int payloadSize = payload.size();

  // create msg bytes 
  const int msgSize = _headerSize + payloadSize + 1; // + 1 for NULL at the end of Message
  auto* msg = new BYTE[msgSize];
  memset(msg, 0x00, msgSize);

  // start byte [0]
  msg[0] = 0x0F;
  // size bytes [1-4]
  msg[4] = static_cast<BYTE>(payloadSize & 0xFF);
  msg[3] = static_cast<BYTE>((payloadSize >> 8) & 0xFF);
  msg[2] = static_cast<BYTE>((payloadSize >> 16) & 0xFF);
  msg[1] = static_cast<BYTE>((payloadSize >> 24) & 0xFF);

  // type byte [5]
  msg[5] = 0x01; // JSON string

  // copy string to msg
  memcpy(msg + _headerSize, payload.c_str(), payloadSize);

  // For Debugging print Header hex values
  // for (int i = 0; i < 30; ++i) {
  //   std::cout << "[" << i << "] " <<
  //     " 0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(*msg) << 
  //     std::dec << " (" << static_cast<int>(*msg) << ")" << std::endl;
  //   msg++;
  // }
  // msg -= 30;

  return {msg, msgSize};
}
