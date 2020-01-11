#pragma once

#include <tuple>
#include "icam.h"


namespace data_reader {
  // Base Cam implementation with some implementation needed for all cameras
  // Every new camera should inherit from the BaseCam
  class BaseCam : public ICam {
  public:
    BaseCam(const std::string name);

    std::tuple<const bool, const int64, cv::Mat> getFrame() override;

    const cv::Size getFrameSize() const override { return _frameSize; }
    const double getFrameRate() const override { return _frameRate; }
    const std::string getName() const override { return _name; }

  protected:
    const std::string _name;
    double _frameRate;
    cv::Size _frameSize;

    cv::Mat _currFrame;
    int64_t _currTs;
    bool _validFrame;
  };
}
