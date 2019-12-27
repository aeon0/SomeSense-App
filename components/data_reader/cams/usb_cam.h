#pragma once

#include <tuple>
#include "icam.h"


namespace data_reader {
  class UsbCam : public ICam {
  public:
    UsbCam(const int deviceIdx);
    virtual std::tuple<const bool, const int64, cv::Mat> getFrame(
      const std::chrono::time_point<std::chrono::high_resolution_clock>& algoStartTime,
      const int64 jumpToTs = -1) override;
    const double getFrameRate() const override { return _frameRate; }

  private:
    cv::VideoCapture _cam;
    double _frameRate;
  };
}
