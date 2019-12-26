#include "video_cam.h"
#include <iostream>
#include <algorithm>


data_reader::VideoCam::VideoCam(const std::string& filename) : _filename(filename) {
  _stream.open(_filename);
  if (!_stream.isOpened()) {
    throw std::runtime_error("VideoCam could not open file: " + _filename);
  }
  _frameRate = _stream.get(cv::CAP_PROP_FPS);
  double frameCount = _stream.get(cv::CAP_PROP_FRAME_COUNT);
  _recLength = static_cast<int64>(((frameCount - 1)/_frameRate) * 1000000);
}

std::tuple<const bool, const int64, cv::Mat> data_reader::VideoCam::getFrame(const int64 jumpToTs) {
  if (jumpToTs > -1) {
    double newTs = static_cast<double>(jumpToTs) / 1000; // in [ms]
    const double lastPossibleTs = _recLength / 1000.0;
    newTs = std::clamp<double>(newTs, 0.0, lastPossibleTs);
    _stream.set(cv::CAP_PROP_POS_MSEC, newTs);
  }
  const double tsMsec = _stream.get(cv::CAP_PROP_POS_MSEC);
  const int64 tsUsec = static_cast<int64>(tsMsec * 1000.0);
  cv::Mat frame;
  const bool success = _stream.read(frame);
  return {success, tsUsec, frame};
}
