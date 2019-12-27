#pragma once

#include <tuple>
#include "icam.h"


namespace data_reader {
  class UsbCam : public ICam {
  public:
    UsbCam(const int deviceIdx, const std::string name);

    std::tuple<const bool, const int64, cv::Mat> getNewFrame(
      const std::chrono::time_point<std::chrono::high_resolution_clock>& algoStartTime,
      const int64 jumpToTs = -1) override;
    std::tuple<const bool, const int64, cv::Mat> getFrame() override;

    const cv::Size getFrameSize() const override { return cv::Size(_cam.get(cv::CAP_PROP_FRAME_WIDTH), _cam.get(cv::CAP_PROP_FRAME_HEIGHT)); }
    const double getFrameRate() const override { return _frameRate; }
    const std::string getName() const override { return _name; }

  private:
    const std::string _name;
    cv::VideoCapture _cam;
    double _frameRate;
    int _deviceIdx;

    cv::Mat _currFrame;
    int64 _currTs;
    bool _validFrame;
  };
}
