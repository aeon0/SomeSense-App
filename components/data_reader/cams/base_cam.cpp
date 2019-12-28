#include "base_cam.h"

data_reader::BaseCam::BaseCam(const std::string name) : 
  _name(name), _currTs(-1), _validFrame(false) {}

std::tuple<const bool, const int64, cv::Mat> data_reader::BaseCam::getFrame() {
  bool success = false;
  if (_currTs >= 0 && _validFrame) {
    success = true;
  }
  return {success, _currTs, _currFrame};
}
