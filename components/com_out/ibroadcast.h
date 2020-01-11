#pragma once

#include <string>

typedef unsigned char BYTE;

// Interface used to send bottom up data from server to clients
// Server class has to implement the interface
namespace com_out {
  class IBroadcast {
  public:
    // boradcast a string of data (e.g. a json string)
    // virtual bool broadcast(const BYTE* buf, const int len) const = 0;
    virtual void broadcast(const std::string payload) const = 0;
  };
}
