#include "app_state.h"
#include <iostream>


serialize::AppState::AppState() : _messagePtr(nullptr) {}

void serialize::AppState::set(std::unique_ptr<capnp::MallocMessageBuilder> messagePtr) {
  std::lock_guard<std::mutex> lockGuard(_stateLock);
  _messagePtr = std::move(messagePtr);
}

bool serialize::AppState::writeToStream(kj::VectorOutputStream& stream) {
  std::lock_guard<std::mutex> lockGuard(_stateLock);
  if (_messagePtr != nullptr) {
    writePackedMessage(stream, *_messagePtr);
    return true;
  }
  return false;
}

bool serialize::AppState::writeToFile(const int fd) {
  std::lock_guard<std::mutex> lockGuard(_stateLock);
  if (_messagePtr != nullptr) {
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

bool serialize::AppState::setRecState(bool isARecording, int64_t recLength, bool isPlaying) {
  std::lock_guard<std::mutex> lockGuard(_stateLock);
  if (_messagePtr != nullptr) {
    auto builder = _messagePtr->getRoot<CapnpOutput::Frame>().getRecState();
    builder.setIsARecording(isARecording);
    builder.setRecLength(recLength);
    builder.setIsPlaying(isPlaying);
    return true;
  }
  return false;
}

bool serialize::AppState::setSaveToFileState(bool isStoring) {
  std::lock_guard<std::mutex> lockGuard(_stateLock);
  if (_messagePtr != nullptr) {
    auto builder = _messagePtr->getRoot<CapnpOutput::Frame>().getSaveToFileState();
    builder.setIsStoring(isStoring);
    return true;
  }
  return false;
}