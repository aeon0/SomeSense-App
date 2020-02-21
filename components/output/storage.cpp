#include "storage.h"
#include <mutex>


std::mutex outputStateLock;
std::mutex camImgsLock;

void output::Storage::set(Frame frame) {
  std::lock_guard<std::mutex> lockGuard(outputStateLock);
  _frameDataJson = frame;
  _frameData = frame;
}

void output::Storage::set(ControlData controlData) {
  std::lock_guard<std::mutex> lockGuard(outputStateLock);
  _controlDataJson = controlData;
  _controlData = controlData;
}

output::Frame output::Storage::getFrame() const {
  std::lock_guard<std::mutex> lockGuard(outputStateLock);
  return _frameData;
}

output::ControlData output::Storage::getControlData() const {
  std::lock_guard<std::mutex> lockGuard(outputStateLock);
  return _controlData;
}

nlohmann::json output::Storage::getFrameJson() const {
  std::lock_guard<std::mutex> lockGuard(outputStateLock);
  return _frameDataJson;
}

nlohmann::json output::Storage::getControlDataJson() const {
  std::lock_guard<std::mutex> lockGuard(outputStateLock);
  return _controlDataJson;
}

int64_t output::Storage::getAlgoTs() const {
  std::lock_guard<std::mutex> lockGuard(outputStateLock);
  return _frameData.timestamp;
}

void output::Storage::setCamImg(std::string key, CamImg data) {
  std::lock_guard<std::mutex> lockGuard(camImgsLock);
  if (_camImgs.find(key) != _camImgs.end()) {
    _camImgs.at(key).img.release();
    _camImgs.at(key) = data;
  }
  else {
    _camImgs.insert({key, data});
  }
}

void output::Storage::getCamImgs(CamImgMap& camImgMap) const {
  std::lock_guard<std::mutex> lockGuard(camImgsLock);
  for (auto [key, data] : _camImgs) {
    camImgMap.insert({key, data});
    camImgMap.at(key).img = data.img.clone();
  }
}
