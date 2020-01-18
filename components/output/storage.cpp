#include "storage.h"
#include <mutex>


std::mutex outputStateLock;

void output::Storage::set(Frame frame) {
  std::lock_guard<std::mutex> lockGuard(outputStateLock);
  nlohmann::json j = frame;
  _frameDataJsonStr = j.dump();
  _frameData = frame;
}

output::Frame output::Storage::get() {
  std::lock_guard<std::mutex> lockGuard(outputStateLock);
  return _frameData;
}

std::string output::Storage::getJsonStr() {
  std::lock_guard<std::mutex> lockGuard(outputStateLock);
  return _frameDataJsonStr;
}
