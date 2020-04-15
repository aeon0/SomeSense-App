#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#include "output/frame.capnp.h"
#include "utilities/json.hpp"


namespace output {
  class Storage {
  public:
    Storage();

    int64_t getAlgoTs();
    void set(std::unique_ptr<capnp::MallocMessageBuilder> messagePtr);
    bool writeToStream(kj::VectorOutputStream& outputStream);
    bool writeToFile(const int fd);

    bool setRecCtrlData(bool isRecording, bool isPlaying, int64_t recLength);
    bool setStoreCtrlData(bool isStoring);

  private:
    std::unique_ptr<capnp::MallocMessageBuilder> _messagePtr;
    std::mutex _outputStateLock;
  };
}
