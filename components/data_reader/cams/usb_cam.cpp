#include "usb_cam.h"
#include <iostream>
#include <thread>


data_reader::UsbCam::UsbCam(
  const std::string name,
  const TS& algoStartTime,
  const int deviceIdx,
  const int captureWidth,
  const int captureHeight,
  const double horizontalFov
):
  BaseCam(name, algoStartTime),
  _deviceIdx(deviceIdx)
{
  _cam.open(_deviceIdx, cv::CAP_V4L);
  if (!_cam.isOpened()) {
    throw std::runtime_error("Could not open USB Camera at index: " + std::to_string(_deviceIdx));
  }

  _cam.set(cv::CAP_PROP_FRAME_WIDTH, captureWidth);
  _cam.set(cv::CAP_PROP_FRAME_HEIGHT, captureHeight);
  _frameRate = _cam.get(cv::CAP_PROP_FPS);
  setIntrinsics(_cam.get(cv::CAP_PROP_FRAME_WIDTH), _cam.get(cv::CAP_PROP_FRAME_HEIGHT), horizontalFov);
  setExtrinsics(0.0, 0.0, 1.7, 0.002, 0.0, 0.0);
}

void data_reader::UsbCam::start() {
  // Start thread to read image and store it into _currFrame
  std::thread dataReaderThread(&data_reader::UsbCam::readData, this);
  dataReaderThread.detach();
}

void data_reader::UsbCam::readData() {
  for (;;) {
    const auto captureTime = std::chrono::high_resolution_clock::now();
    bool success = _cam.read(_bufferFrame);

    std::lock_guard<std::mutex> lockGuard(_readMutex);
    _currTs = static_cast<int64_t>(std::chrono::duration<double, std::micro>(captureTime - _algoStartTime).count());
    _currFrame = _bufferFrame.clone();
    _bufferFrame.release();
    _validFrame = success;
  }
}
