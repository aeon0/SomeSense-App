#include "storage.h"
#include <mutex>


std::mutex outputStateLock;

void output::Storage::set(Frame frame) {
  std::lock_guard<std::mutex> lockGuard(outputStateLock);
  _frameDataJson = frame;
  _frameData = frame;
}

output::Frame output::Storage::get() const {
  std::lock_guard<std::mutex> lockGuard(outputStateLock);
  return _frameData;
}

nlohmann::json output::Storage::getJson() const {
  std::lock_guard<std::mutex> lockGuard(outputStateLock);
  return _frameDataJson;
}

int64_t output::Storage::getAlgoTs() const {
  std::lock_guard<std::mutex> lockGuard(outputStateLock);
  return _frameData.timestamp;
}