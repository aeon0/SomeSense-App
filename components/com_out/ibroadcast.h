#pragma once

#include <string>

typedef unsigned char BYTE;

// Interface used to send bottom up data from server to clients
// Server class has to implement the interface
namespace com_out {
  class IBroadcast {
  public:
    virtual void broadcast(const std::string payload) const = 0;
    virtual void broadcast(int sensorIdx, BYTE* payload, int width, int height, int channels, int64_t ts) const = 0;
  };
}
