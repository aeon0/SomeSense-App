#include "base_cam.h"

data_reader::BaseCam::BaseCam(const std::string name, const TS& algoStartTime) : 
  _name(name), _algoStartTime(algoStartTime), _currTs(-1), _currFrameNr(-1), _validFrame(false) {}

std::tuple<const bool, const int64_t, cv::Mat> data_reader::BaseCam::getFrame() {
  std::lock_guard<std::mutex> lockGuard(_readMutex);
  bool success = false;
  if (_currTs >= 0 && _currFrameNr >= 0 && _validFrame) {
    success = true;
  }
  // TODO: maybe speed up possible with memcopy just data
  return {success, _currTs, _currFrame.clone() };
}

void data_reader::BaseCam::setCamIntrinsics(const int width, const int height, const double horizontalFov) {
  assert(horizontalFov > 0.0);
  assert(width > 0);
  assert(height > 0);

  // Set frame size
  _frameSize = cv::Size(width, height);
  // Set principal point
  _cx = static_cast<double>(_frameSize.width) / 2.0;
  _cy = static_cast<double>(_frameSize.height) / 2.0;
  // Set field of view
  _horizontalFov = horizontalFov;
  _verticalFov = (static_cast<double>(_frameSize.height) *_horizontalFov) / static_cast<double>(_frameSize.width);
  // Set focal length
  _fx = (static_cast<double>(width) * 0.5) / tan(_horizontalFov * 0.5);
  _fx = _fy; // assuming concentric lens
}
