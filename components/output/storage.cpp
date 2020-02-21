#include "storage.h"
#include <mutex>


std::mutex outputStateLock;
std::mutex camImgsLock;

void output::Storage::set(Frame frame) {
  std::lock_guard<std::mutex> lockGuard(outputStateLock);
  _frameDataJson = frame;
  _frameData = frame;
}

void output::Storage::set(CtrlData ctrlData) {
  std::lock_guard<std::mutex> lockGuard(outputStateLock);
  _ctrlDataJson = ctrlData;
  _ctrlData = ctrlData;
}

output::Frame output::Storage::getFrame() const {
  std::lock_guard<std::mutex> lockGuard(outputStateLock);
  return _frameData;
}

output::CtrlData output::Storage::getCtrlData() const {
  std::lock_guard<std::mutex> lockGuard(outputStateLock);
  return _ctrlData;
}

nlohmann::json output::Storage::getFrameJson() const {
  std::lock_guard<std::mutex> lockGuard(outputStateLock);
  return _frameDataJson;
}

nlohmann::json output::Storage::getCtrlDataJson() const {
  std::lock_guard<std::mutex> lockGuard(outputStateLock);
  return _ctrlDataJson;
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
