#include "usb_cam.h"
#include <iostream>


data_reader::UsbCam::UsbCam(const int deviceIdx, const std::string name):
    _deviceIdx(deviceIdx), _name(name), _currTs(-1), _validFrame(false) {
  _cam.open(_deviceIdx);
  if (!_cam.isOpened()) {
    throw std::runtime_error("Could not open USB Camera at index: " + _deviceIdx);
  }
  _frameRate = _cam.get(cv::CAP_PROP_FPS);
}

std::tuple<const bool, const int64, cv::Mat> data_reader::UsbCam::getNewFrame(
      const std::chrono::time_point<std::chrono::high_resolution_clock>& algoStartTime,
      const int64 currentAlgoTs,
      const bool updateToAlgoTs) {
  static_cast<void>(currentAlgoTs);
  static_cast<void>(updateToAlgoTs);

  const auto captureTime = std::chrono::high_resolution_clock::now();
  _validFrame = _cam.read(_currFrame);
  // const auto endCaptureTime = std::chrono::high_resolution_clock::now();
  // auto _duration = std::chrono::duration<double, std::milli>(endCaptureTime - captureTime);
  // std::cout << std::fixed << std::setprecision(2) << "GetFrame: " << _duration.count() << " ms" << std::endl;
  _currTs = static_cast<int64>(std::chrono::duration<double, std::micro>(captureTime - algoStartTime).count());
  return {_validFrame, _currTs, _currFrame};
}

std::tuple<const bool, const int64, cv::Mat> data_reader::UsbCam::getFrame() {
  bool success = false;
  if (_currTs >= 0 && _validFrame) {
    success = true;
  }
  return {success, _currTs, _currFrame};
}
