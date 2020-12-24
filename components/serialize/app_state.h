#pragma once

#include <string>
#include <vector>
#include <atomic>
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

    bool setRecState(bool isARecording, int64_t recLength, bool isPlaying);
    bool setSaveToFileState(bool isStoring);
    bool isDirty() { return _isDirty; }

  private:
    std::unique_ptr<capnp::MallocMessageBuilder> _messagePtr;
    std::mutex _stateLock;
    std::atomic<bool> _isInit;
    std::atomic<bool> _isDirty;
  };
}
