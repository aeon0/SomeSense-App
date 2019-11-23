#include "video_cam.h"

data_reader::VideoCam::VideoCam(const std::string& filename) : _filename(filename) {
  _stream.open(_filename);
  if (!_stream.isOpened()) {
    throw std::runtime_error("VideoCam could not open file: " + _filename);
  }
}

cv::Mat data_reader::VideoCam::getFrame() {
  cv::Mat frame;
  _stream.read(frame);
  return frame;
}
