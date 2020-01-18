#include "storage.h"
#include <mutex>


std::mutex outputStateLock;
std::mutex camImgsLock;

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

void output::Storage::setCamImg(std::string key, CamImg data) {
  std::lock_guard<std::mutex> lockGuard(camImgsLock);
  if (_camImgs.find(key) != _camImgs.end()) {
    _camImgs.at(key) = data;
  }
  else {
    _camImgs.insert({key, data});
  }
}

output::Storage::CamImgMap output::Storage::getCamImgs() const {
  std::lock_guard<std::mutex> lockGuard(camImgsLock);
  return _camImgs;
}
