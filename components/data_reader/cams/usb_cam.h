#pragma once

#include <tuple>
#include "base_cam.h"


namespace data_reader {
  class UsbCam : public BaseCam {
  public:
    UsbCam(const int deviceIdx, const std::string name);

    std::tuple<const bool, const int64, cv::Mat> getNewFrame(
      const std::chrono::time_point<std::chrono::high_resolution_clock>& algoStartTime,
      const int64 currentAlgoTs,
      const bool updateToAlgoTs) override;

  private:
    cv::VideoCapture _cam;
    int _deviceIdx;
  };
}
