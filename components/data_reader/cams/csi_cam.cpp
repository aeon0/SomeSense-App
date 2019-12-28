#include "csi_cam.h"
#include <iostream>


data_reader::CsiCam::CsiCam(const std::string name, int captureWidth, int captureHeight, int displayWidth,
                            int displayHeight, double frameRate, int flipMethod): BaseCam(name) {

  std::string gsStreamerPipline = 
    "nvarguscamerasrc ! video/x-raw(memory:NVMM), width=(int)" + std::to_string(captureWidth) + ", height=(int)" +
    std::to_string(captureHeight) + ", format=(string)NV12, framerate=(fraction)" + std::to_string(frameRate) +
    "/1 ! nvvidconv flip-method=" + std::to_string(flipMethod) + " ! video/x-raw, width=(int)" + std::to_string(displayWidth) + ", height=(int)" +
    std::to_string(displayHeight) + ", format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! appsink";

  std::cout << gsStreamerPipline << std::endl;

  _cam.open(gsStreamerPipline, cv::CAP_GSTREAMER);

  if (!_cam.isOpened()) {
    throw std::runtime_error("Could not open CSI Camera: " + name);
  }
  _frameRate = _cam.get(cv::CAP_PROP_FPS);
  _frameSize = cv::Size(_cam.get(cv::CAP_PROP_FRAME_WIDTH), _cam.get(cv::CAP_PROP_FRAME_HEIGHT));
}

std::tuple<const bool, const int64, cv::Mat> data_reader::CsiCam::getNewFrame(
      const std::chrono::time_point<std::chrono::high_resolution_clock>& algoStartTime,
      const int64 currentAlgoTs,
      const bool updateToAlgoTs) {
  static_cast<void>(currentAlgoTs);
  static_cast<void>(updateToAlgoTs);

  const auto captureTime = std::chrono::high_resolution_clock::now();
  _validFrame = _cam.read(_currFrame); // Reading takes around 2-3 ms on the Jetson Nano

_currTs = static_cast<int64>(std::chrono::duration<double, std::micro>(captureTime - algoStartTime).count());
  return {_validFrame, _currTs, _currFrame};
}
