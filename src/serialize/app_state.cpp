#include "app_state.h"
#include <iostream>


serialize::AppState::AppState() : _messagePtr(nullptr), _isInit(false) {
  _messagePtr = std::make_unique<capnp::MallocMessageBuilder>();
  _messagePtr->initRoot<CapnpOutput::Frame>();

  _ctrlData["isPlaying"] = false;
  _ctrlData["recLength"] = 0;
  _ctrlData["isARecording"] = false;
  _ctrlData["isStoring"] = false;

  _isDirty = false;
}

void serialize::AppState::setFrame(std::unique_ptr<capnp::MallocMessageBuilder> messagePtr) {
  std::unique_lock<std::mutex> uniqueLock(_stateLock);
  _messagePtr = std::move(messagePtr);
  _isInit = true;
  _isDirty = true;
}

bool serialize::AppState::writeToStream(kj::VectorOutputStream& stream) {
  std::lock_guard<std::mutex> lockGuard(_stateLock);
  if (_messagePtr != nullptr && _isInit) {
    writePackedMessage(stream, *_messagePtr);
    _isDirty = false;
    return true;
  }
  return false;
}

bool serialize::AppState::writeToFile(const int fd) {
  std::lock_guard<std::mutex> lockGuard(_stateLock);
  if (_messagePtr != nullptr && _isInit) {
    writePackedMessageToFd(fd, *_messagePtr);
    return true;
  }
  return false;
}

int64_t serialize::AppState::getAlgoTs() {
  std::lock_guard<std::mutex> lockGuard(_stateLock);
  if (_messagePtr != nullptr) {
    return _messagePtr->getRoot<CapnpOutput::Frame>().getTimestamp();
  }
  return -1;
}

void serialize::AppState::setRecData(bool isPlaying, int recLength, bool isARecording, bool makeDirty) {
  _ctrlData["isPlaying"] = isPlaying;
  _ctrlData["recLength"] = recLength;
  _ctrlData["isARecording"] = isARecording;
  if (makeDirty) {
    _isDirty = true;
  }
}

void serialize::AppState::setStoringData(bool isStoring, bool makeDirty) {
  _ctrlData["isStoring"] = isStoring;
  if (makeDirty) {
    _isDirty = true;
  }
}
