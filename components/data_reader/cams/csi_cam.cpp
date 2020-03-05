#include "csi_cam.h"
#include <iostream>
#include <thread>


std::string gstreamer_pipeline(int captureWidth, int captureHeight, int frameRate, int flipMethod) {
  return "nvarguscamerasrc ! video/x-raw(memory:NVMM), width=(int)" + std::to_string(captureWidth) + ", height=(int)" +
          std::to_string(captureHeight) + ", format=(string)NV12, framerate=(fraction)" + std::to_string(frameRate) +
          "/1 ! nvvidconv flip-method=" + std::to_string(flipMethod) + " ! videoconvert ! video/x-raw, format=(string)BGR ! appsink";
}


data_reader::CsiCam::CsiCam(const std::string name, const TS& algoStartTime, int captureWidth, int captureHeight, int frameRate, int flipMethod): BaseCam(name, algoStartTime) {
  std::cout << "Creating CSI CAMERA!" << std::endl;
  std::string gsStreamerPipline = gstreamer_pipeline(captureWidth, captureHeight, frameRate, flipMethod);
  std::cout << "Using GStream Pipline: " << gsStreamerPipline << std::endl;

  _cam.open(gsStreamerPipline, cv::CAP_GSTREAMER);
  if (!_cam.isOpened()) {
    throw std::runtime_error("Could not open CSI Camera: " + name);
  }

  _frameRate = _cam.get(cv::CAP_PROP_FPS);
  _frameSize = cv::Size(_cam.get(cv::CAP_PROP_FRAME_WIDTH), _cam.get(cv::CAP_PROP_FRAME_HEIGHT));

  bool success = _cam.read(_bufferFrame);
  std::cout << "Success: " << success << std::endl;

  // Start thread to read image and store it into _currFrame
  std::thread dataReaderThread(&data_reader::CsiCam::readData, this);
  dataReaderThread.detach();
}

void data_reader::CsiCam::readData() {
  for (;;) {
    const auto captureTime = std::chrono::high_resolution_clock::now();
    bool success = _cam.read(_bufferFrame);

    std::lock_guard<std::mutex> lockGuard(_readMutex);
    _currTs = static_cast<int64_t>(std::chrono::duration<double, std::micro>(captureTime - _algoStartTime).count());
    _currFrameNr += 1;
    _currFrame = _bufferFrame.clone();
    _bufferFrame.release();
    _validFrame = success;

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }
}
