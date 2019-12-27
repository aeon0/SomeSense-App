#pragma once

#include <string>

// Interface used to send bottom up data from server to clients
// Server class has to implement the interface
namespace com_out {
  class IBroadcast {
  public:
    virtual bool broadcast(const std::string msg) const = 0;
  };
}
