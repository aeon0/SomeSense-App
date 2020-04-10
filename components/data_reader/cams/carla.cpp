#include "carla.h"
#include <iostream>
#include <thread>


data_reader::Carla::Carla(const std::string name, const TS& algoStartTime):
    BaseCam(name, algoStartTime) {
 
  // _frameRate = _cam.get(cv::CAP_PROP_FPS);
  // _frameSize = cv::Size(_cam.get(cv::CAP_PROP_FRAME_WIDTH), _cam.get(cv::CAP_PROP_FRAME_HEIGHT));

  // Start thread to read image and store it into _currFrame
  std::thread dataReaderThread(&data_reader::Carla::readData, this);
  dataReaderThread.detach();
}

void data_reader::Carla::readData() {
  for (;;) {
    const auto captureTime = std::chrono::high_resolution_clock::now();

    // bool success = _cam.read(_bufferFrame);

    std::lock_guard<std::mutex> lockGuard(_readMutex);
    _currTs = static_cast<int64_t>(std::chrono::duration<double, std::micro>(captureTime - _algoStartTime).count());
    _currFrameNr += 1;
    _currFrame = _bufferFrame.clone();
    _bufferFrame.release();
    _validFrame = true;
  }
}
