#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#include "serialize/frame.capnp.h"
#include "utilities/json.hpp"


namespace serialize {
  class AppState {
  public:
    AppState();

    int64_t getAlgoTs();
    void set(std::unique_ptr<capnp::MallocMessageBuilder> messagePtr);
    bool writeToStream(kj::VectorOutputStream& stream);
    bool writeToFile(const int fd);

  private:
    std::unique_ptr<capnp::MallocMessageBuilder> _messagePtr;
    std::mutex _stateLock;
  };
}
