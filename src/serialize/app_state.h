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
    void setFrame(std::unique_ptr<capnp::MallocMessageBuilder> messagePtr);
    bool writeToStream(kj::VectorOutputStream& stream);
    bool writeToFile(const int fd);
    void setRecData(bool isPlaying, int recLength, bool isARecording = true, bool makeDirty = true);
    void setStoringData(bool isStoring, bool makeDirty = true);

    bool isDirty() const { return _isDirty; }
    const nlohmann::json& getCtrlData() const { return _ctrlData; }

  private:
    nlohmann::json _ctrlData;
    std::unique_ptr<capnp::MallocMessageBuilder> _messagePtr;
    std::mutex _stateLock;
    std::atomic<bool> _isInit;
    std::atomic<bool> _isDirty;
  };
}
