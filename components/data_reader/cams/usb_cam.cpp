#include "usb_cam.h"
#include <iostream>
#include <thread>


data_reader::UsbCam::UsbCam(const std::string name, const TS& algoStartTime, const int deviceIdx):
    BaseCam(name, algoStartTime), _deviceIdx(deviceIdx) {
  _cam.open(_deviceIdx);
  if (!_cam.isOpened()) {
    throw std::runtime_error("Could not open USB Camera at index: " + _deviceIdx);
  }
  _frameRate = _cam.get(cv::CAP_PROP_FPS);
  _frameSize = cv::Size(_cam.get(cv::CAP_PROP_FRAME_WIDTH), _cam.get(cv::CAP_PROP_FRAME_HEIGHT));

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
    _currFrameNr += 1;
    _currFrame = _bufferFrame.clone();
    _bufferFrame.release();
    _validFrame = success;

    // Not sure why I need this... but it seems otherwise the readFrame() is not fast enough
    // to take the lock...
    // std::this_thread::sleep_for(std::chrono::microseconds(100));
  }
}
