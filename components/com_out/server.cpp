#include "server.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <thread>
#include <chrono>
#include "utilities/json.hpp"


com_out::Server::Server(const output::Storage& outputStorage) :
  _outputStorage(outputStorage), _lastSentTs(-1), _pollOutput(true) {
  _buf = new char[1024];
}

com_out::Server::~Server() {
  delete _buf;
}

void com_out::Server::run() {
  create();
  serve();
}

void com_out::Server::stop() {
  close(_server);
  _pollOutput = false;
}

void com_out::Server::pollOutput() {
  while(_pollOutput) {
    int64_t currAlgoTs = _outputStorage.getAlgoTs();
    if (currAlgoTs > _lastSentTs) {
      _lastSentTs = currAlgoTs;

      // Send raw sensor data (has to be before algo data!)
     output::Storage::CamImgMap camImgMap;
     _outputStorage.getCamImgs(camImgMap);
      for (auto [key, data] : camImgMap) {
        broadcast(
          data.sensorIdx,
          data.img.data,
          data.width,
          data.height,
          data.channels,
          data.timestamp
        );
        data.img.release();
      }

      // Send algo data
      nlohmann::json out {
        {"type", "server.frame"},
        {"data", _outputStorage.getJson()}
      };
      broadcast(out.dump());
    }
    // Polling every 2 ms to check if there is new data
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
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
    try {
      if((nwritten = send(client, buf, nleft, 0)) < 0) {
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

void com_out::Server::broadcast(int sensorIdx, BYTE* payload, int width, int height, int channels, int64_t ts) const {
  auto [msg, len] = createMsg(sensorIdx, payload, width, height, channels, ts);
  for(int client: _clients) {
    sendToClient(client, msg, len);
  }
  delete [] msg;
}

std::tuple<BYTE*, int> com_out::Server::createMsg(int sensorIdx, BYTE* payload, int width, int height, int channels, int64_t ts) const {
  const int payloadSize = width * height * channels;

  // create msg bytes 
  const int msgSize = _headerSize + payloadSize;
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
  msg[5] = 0x10; // Raw Image

  // add image dimensions
  assert(width <= 0xFFFF && "width is too large");
  assert(height <= 0xFFFF && "height is too large");
  assert(channels <= 0xFF && "channels are too large");
  // width [6-7]
  msg[7] = static_cast<BYTE>(width & 0xFF);
  msg[6] = static_cast<BYTE>((width >> 8) & 0xFF);
  // height [8-9]
  msg[9] = static_cast<BYTE>(height & 0xFF);
  msg[8] = static_cast<BYTE>((height >> 8) & 0xFF);
  // channels [10]
  msg[10] = static_cast<BYTE>(channels & 0xFF);

  // image timestamp [11-18]
  msg[18] = static_cast<BYTE>(ts & 0xFF);
  msg[17] = static_cast<BYTE>((ts >> 8) & 0xFF);
  msg[16] = static_cast<BYTE>((ts >> 16) & 0xFF);
  msg[15] = static_cast<BYTE>((ts >> 24) & 0xFF);
  msg[14] = static_cast<BYTE>((ts >> 32) & 0xFF);
  msg[13] = static_cast<BYTE>((ts >> 38) & 0xFF);
  msg[12] = static_cast<BYTE>((ts >> 46) & 0xFF);
  msg[11] = static_cast<BYTE>((ts >> 54) & 0xFF);

  // sensor idx [19]
  assert(sensorIdx <= 0xFF && "sensorIdx too large");
  msg[19] = static_cast<BYTE>(sensorIdx & 0xFF);

  // copy img data to msg
  memcpy(msg + _headerSize, payload, payloadSize);

  // For Debugging print Header hex values
  // for (int i = 0; i < _headerSize; ++i) {
  //   std::cout << "[" << i << "] " <<
  //     " 0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(*msg) << 
  //     std::dec << " (" << static_cast<int>(*msg) << ")" << std::endl;
  //   msg++;
  // }
  // msg -= _headerSize;

  return {msg, msgSize};
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
  const int msgSize = _headerSize + payloadSize;
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

  return {msg, msgSize};
}
