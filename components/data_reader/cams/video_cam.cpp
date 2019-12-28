#include "video_cam.h"
#include <iostream>
#include <algorithm>


data_reader::VideoCam::VideoCam(const std::string& filename, const std::string name, const std::vector<int64> timestamps) :
    _filename(filename), _name(name), _timestamps(timestamps), _currTs(-1), _validFrame(false) {
  _stream.open(_filename);
  if (!_stream.isOpened()) {
    throw std::runtime_error("VideoCam could not open file: " + _filename);
  }
  _frameRate = _stream.get(cv::CAP_PROP_FPS);
  double frameCount = _stream.get(cv::CAP_PROP_FRAME_COUNT);
  _recLength = static_cast<int64>(((frameCount - 1)/_frameRate) * 1000000);
}

std::tuple<const bool, const int64, cv::Mat> data_reader::VideoCam::getNewFrame(
      const std::chrono::time_point<std::chrono::high_resolution_clock>& algoStartTime,
      const int64 currentAlgoTs,
      const bool updateToAlgoTs) {
  // TODO: use the _timestamps if filled to check for specific frames timestamp... etc.
  static_cast<void>(algoStartTime);
  if (updateToAlgoTs > -1) {
    double newTs = static_cast<double>(currentAlgoTs) / 1000; // in [ms]
    const double lastPossibleTs = _recLength / 1000.0;
    newTs = std::clamp<double>(newTs, 0.0, lastPossibleTs);
    _stream.set(cv::CAP_PROP_POS_MSEC, newTs);
  }
  const double tsMsec = _stream.get(cv::CAP_PROP_POS_MSEC);
  _currTs = static_cast<int64>(tsMsec * 1000.0);
  _validFrame = _stream.read(_currFrame);
  return {_validFrame, _currTs, _currFrame};
}

std::tuple<const bool, const int64, cv::Mat> data_reader::VideoCam::getFrame() {
  bool success = false;
  if (_currTs >= 0 && _validFrame) {
    success = true;
  }
  return {success, _currTs, _currFrame};
}
