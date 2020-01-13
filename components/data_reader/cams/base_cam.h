#pragma once

#include <tuple>
#include <mutex>
#include "icam.h"
#include "../types.h"


namespace data_reader {
  // Base Cam implementation with some implementation needed for all cameras
  // Every new camera should inherit from the BaseCam
  class BaseCam : public ICam {
  public:
    BaseCam(const std::string name, const TS& algoStartTime);

    std::tuple<const bool, const int64_t, cv::Mat> getFrame() override;

    const cv::Size getFrameSize() const override { return _frameSize; }
    const double getFrameRate() const override { return _frameRate; }
    const std::string getName() const override { return _name; }

  protected:
    const std::string _name;
    double _frameRate;
    cv::Size _frameSize;

    cv::Mat _bufferFrame;
    cv::Mat _currFrame;
    int64_t _currTs;
    bool _validFrame;

    const TS& _algoStartTime;

    std::mutex _readMutex;
  };
}
