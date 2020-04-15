#include "storage.h"
#include <iostream>


output::Storage::Storage() : _messagePtr(nullptr) {}

void output::Storage::set(std::unique_ptr<capnp::MallocMessageBuilder> messagePtr) {
  std::lock_guard<std::mutex> lockGuard(_outputStateLock);
  _messagePtr = std::move(messagePtr);
}

bool output::Storage::writeToStream(kj::VectorOutputStream& outputStream) {
  std::lock_guard<std::mutex> lockGuard(_outputStateLock);
  if (_messagePtr != nullptr) {
    writePackedMessage(outputStream, *_messagePtr);
    return true;
  }
  return false;
}

bool output::Storage::writeToFile(const int fd) {
  std::lock_guard<std::mutex> lockGuard(_outputStateLock);
  if (_messagePtr != nullptr) {
    writePackedMessageToFd(fd, *_messagePtr);
    return true;
  }
  return false;
}

int64_t output::Storage::getAlgoTs() {
  std::lock_guard<std::mutex> lockGuard(_outputStateLock);
  if (_messagePtr != nullptr) {
    return _messagePtr->getRoot<CapnpOutput::Frame>().getTimestamp();
  }
  return -1;
}

bool output::Storage::setRecCtrlData(bool isARecording, bool isPlaying, int64_t recLength) {
  std::lock_guard<std::mutex> lockGuard(_outputStateLock);
  if (_messagePtr != nullptr) {
    _messagePtr->getRoot<CapnpOutput::Frame>().getCtrlData().setIsARecording(isARecording);
    _messagePtr->getRoot<CapnpOutput::Frame>().getCtrlData().setIsPlaying(isPlaying);
    _messagePtr->getRoot<CapnpOutput::Frame>().getCtrlData().setRecLength(recLength);
    return true;
  }
  return false;
}

bool output::Storage::setStoreCtrlData(bool isStoring) {
  std::lock_guard<std::mutex> lockGuard(_outputStateLock);
  if (_messagePtr != nullptr) {
    _messagePtr->getRoot<CapnpOutput::Frame>().getCtrlData().setIsStoring(isStoring);
    return true;
  }
  return false;
}
