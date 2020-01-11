#include "video_cam.h"
#include <iostream>
#include <algorithm>


data_reader::VideoCam::VideoCam(const std::string name, const std::string& filename, const std::vector<int64> timestamps) :
    BaseCam(name), _filename(filename), _timestamps(timestamps) {
  _stream.open(_filename);
  if (!_stream.isOpened()) {
    throw std::runtime_error("VideoCam could not open file: " + _filename);
  }
  _frameRate = _stream.get(cv::CAP_PROP_FPS);
  double frameCount = _stream.get(cv::CAP_PROP_FRAME_COUNT);
  _recLength = static_cast<int64>(((frameCount - 1)/_frameRate) * 1000000);
  _frameSize = cv::Size(_stream.get(cv::CAP_PROP_FRAME_WIDTH), _stream.get(cv::CAP_PROP_FRAME_HEIGHT));
}

std::tuple<const bool, const int64, cv::Mat> data_reader::VideoCam::getNewFrame(
      const std::chrono::time_point<std::chrono::high_resolution_clock>& algoStartTime,
      const int64_t currentAlgoTs,
      const bool updateToAlgoTs) {

  const auto captureTime = std::chrono::high_resolution_clock::now();

  // TODO: 1) Check if _timestamps is filled, if it is:
  //       2) Find all ts in _timestamps that are between ]_currTs, currentAlgoTs]
  //       3) If non is found: No frame can be grabbed
  //       4) If one or multiple is found: grab the frame by reading as many as found e.g.
  //          3 frames found, call _stream.read() 3x. Return the frame and set _currTs to the last found

  // TODO: How to handle updateToAlgoTs??

  if (updateToAlgoTs) {
    double newTs = static_cast<double>(currentAlgoTs) / 1000; // in [ms]
    const double lastPossibleTs = _recLength / 1000.0;
    newTs = std::clamp<double>(newTs, 0.0, lastPossibleTs);
    _stream.set(cv::CAP_PROP_POS_MSEC, newTs);
  }

  const double tsMsec = _stream.get(cv::CAP_PROP_POS_MSEC);
  _currTs = static_cast<int64>(tsMsec * 1000.0);
  _validFrame = _stream.read(_currFrame);

  const auto endCaptureTime = std::chrono::high_resolution_clock::now();
  auto _duration = std::chrono::duration<double, std::milli>(endCaptureTime - captureTime);
  // std::cout << std::fixed << std::setprecision(2) << "GetFrame: " << _duration.count() << " ms" << std::endl;

  return {_validFrame, _currTs, _currFrame};
}
