#include "tcp_server.h"
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <thread>

#include <iostream>
#include "util/json.hpp"

// #include <iomanip>
// #include <algorithm>
// #include <kj/io.h>
// #include <signal.h>
// #include <stdlib.h>
// #include <stdio.h>
// #include <sys/un.h>
// #include <sys/types.h>


frame::TcpServer::TcpServer() {
  std::cout << "[TcpServer] Listening at Port: " << _PORT << std::endl;
  _buf = new char[_BUFFER_SIZE];

  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(_PORT);
  
  // _server = socket(AF_INET, SOCK_DGRAM, 0); // TCP
  if ((_server = socket(AF_INET, SOCK_STREAM, 0)) < 0) throw std::runtime_error("Socket creation failed");

  const int sockOpt = 1; 
  if (setsockopt(_server, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &sockOpt, sizeof(sockOpt))) {
    throw std::runtime_error("Error on setting socket options");
  }

  // // This option is used to reduce latency by forcing every message to start with its own tcp package
  // const int tcpOpt = 1;
  // if (setsockopt(_server, SOL_TCP, TCP_NODELAY, &tcpOpt, sizeof(tcpOpt))) {
  //   throw std::runtime_error("Error on setting tcp options");
  // }

  // call bind to associate the socket with the UNIX file system
  if(bind(_server, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
    throw std::runtime_error("Error on socket binding");
  }

  // convert the socket to listen for incoming connections
  if(listen(_server, SOMAXCONN) < 0) {
    throw std::runtime_error("Error on socket listen");
  }
  
  // TODO: Do we need this in a sepreate thread?, I think so
  _thread = std::thread(&frame::TcpServer::serve, this);
}

frame::TcpServer::~TcpServer() {
  delete _buf;
  close(_server);
  _thread.detach();
}

void frame::TcpServer::serve() {
  int client;
  struct sockaddr_in clientAddr;
  socklen_t clientlen = sizeof(clientAddr);

  while((client = accept(_server, (struct sockaddr*)&clientAddr, &clientlen)) > 0) {
    // brackets needed to remove locks after adding to _clients
    {
      std::lock_guard<std::mutex> lockGuard2(_clientsMtx);
      std::cout << "[TcpServe] New client: " << client << std::endl;
      _clients.push_back(client);
    }
    std::thread handleClientThread(&frame::TcpServer::handle, this, client);
    handleClientThread.detach();
  }
}

void frame::TcpServer::handle(int client) {
  for (;;) {
    std::string request = getRequest(client);
    if (request.empty()) {
      break;
    }

    std::string response = "";
    {
      std::lock_guard<std::mutex> lock(_cb_sync);
      if (_cb) (_cb)(request, response);
    }
    // std::cout << "[TcpServer] Request: " << request;
    // std::cout << "[TcpServer] Response: " << response << std::endl;

    auto [msg, len] = createMsg((BYTE*)response.data(), response.size(), frame::JSON);
    const bool success = sendToClient(client, msg, len);
    delete [] msg;
    if (!success) {
      break;
    }
  }

  // remove client
  std::lock_guard<std::mutex> lockGuard(_clientsMtx);
  std::cout << "Remove client: " << client << std::endl;
  _clients.erase(std::remove(_clients.begin(), _clients.end(), client), _clients.end());
  close(client);
}

std::string frame::TcpServer::getRequest(int client) {
  std::string request = "";

  // read until we get a newline
  while(request.find("\n") == std::string::npos) {
    int nread = recv(client, _buf, _BUFFER_SIZE, 0);
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

bool frame::TcpServer::sendToClient(int client, const BYTE* buf, const int len) {
  // prepare to send response
  int nleft = len;
  int nwritten;
  // loop to be sure it is all sent
  while(nleft) {
    try {
      {
        // Needed because otherwise there can be trouble if client is removed while send is active
        std::lock_guard<std::mutex> lockGuard(_clientsMtx);
        nwritten = send(client, buf, nleft, 0);
      }
      if(nwritten < 0) {
        if (errno == EINTR) {
          // the socket call was interrupted -- try again
          continue;
        } 
        else {
          // an error occurred, so break out
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
    catch (const std::exception& e) {
      return false;
    }
  }
  return true;
}

void frame::TcpServer::broadcast(const BYTE* payload, int payloadSize, TcpMessageType type) {
  auto [msg, msgSize] = createMsg(payload, payloadSize, type);
  for(int client: _clients) {
    sendToClient(client, msg, msgSize);
  }
  delete [] msg;
}

std::tuple<BYTE*, int> frame::TcpServer::createMsg(const BYTE* payload, int payloadSize, TcpMessageType type) const {
  // create msg bytes 
  const int msgSize = _HEADER_SIZE + payloadSize;
  auto* msg = new BYTE[msgSize];
  memset(msg, 0x00, msgSize);

  // start bytes [0-1]
  msg[0] = 0x0F;
  msg[1] = 0xF0;
  // size bytes [2-5] in big endian
  msg[5] = static_cast<BYTE>(payloadSize & 0xFF);
  msg[4] = static_cast<BYTE>((payloadSize >> 8) & 0xFF);
  msg[3] = static_cast<BYTE>((payloadSize >> 16) & 0xFF);
  msg[2] = static_cast<BYTE>((payloadSize >> 24) & 0xFF);

  // type byte [6]
  msg[6] = static_cast<BYTE>(type & 0xFF); // proto binary data

  // interface verison byte [7]: major [8]: minor
  msg[7] = 0x00;
  msg[8] = 0x00;

  // copy proto binary data to msg
  memcpy(msg + _HEADER_SIZE, payload, payloadSize);

  return {msg, msgSize};
}


void frame::TcpServer::registerClientCallback(CallbackT cb) {
  std::lock_guard<std::mutex> lock(_cb_sync);
  _cb = cb;
}

void frame::TcpServer::sendFrame(const proto::Frame& msg) {
  sendProto<proto::Frame>(msg, frame::PROTO_FRAME);
}

void frame::TcpServer::syncFrame(const proto::Frame& msg) {
  sendProto<proto::Frame>(msg, frame::PROTO_SYNC);
}

void frame::TcpServer::sendRecMeta(const proto::RecMeta& msg) {
  sendProto<proto::RecMeta>(msg, frame::PROTO_RECMETA);
}
