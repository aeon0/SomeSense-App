#include "base_cam.h"

data_reader::BaseCam::BaseCam(const std::string name, const TS& algoStartTime) : 
  _name(name), _algoStartTime(algoStartTime), _currTs(-1), _validFrame(false) {}

std::tuple<const bool, const int64_t, cv::Mat> data_reader::BaseCam::getFrame() {
  std::lock_guard<std::mutex> lockGuard(_readMutex);
  bool success = false;
  if (_currTs >= 0 && _validFrame) {
    success = true;
  }
  // TODO: maybe speed up possible with memcopy just data
  return {success, _currTs, _currFrame.clone() };
}
