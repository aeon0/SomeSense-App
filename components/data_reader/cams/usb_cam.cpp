#include "usb_cam.h"
#include <iostream>


data_reader::UsbCam::UsbCam(const int deviceIdx) {
  _cam.open(deviceIdx);
  if (!_cam.isOpened()) {
    throw std::runtime_error("Could not open USB Camera at index: " + deviceIdx);
  }
  _frameRate = _cam.get(cv::CAP_PROP_FPS);
}

std::tuple<const bool, const int64, cv::Mat> data_reader::UsbCam::getFrame(
    const std::chrono::time_point<std::chrono::high_resolution_clock>& algoStartTime,
    const int64 jumpToTs) {
  static_cast<void>(jumpToTs);

  const auto captureTime = std::chrono::high_resolution_clock::now();
  cv::Mat frame;
  const bool success = _cam.read(frame);
  const int64 tsUsec = static_cast<int64>(std::chrono::duration<double, std::micro>(algoStartTime - captureTime).count());
  return {success, tsUsec, frame};
}
