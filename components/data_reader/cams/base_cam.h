#pragma once

#include <tuple>
#include <mutex>
#include <atomic>
#include "icam.h"
#include "../types.h"


namespace data_reader {
  // Base Cam implementation with some implementation needed for all cameras
  // Every new camera should inherit from the BaseCam
  class BaseCam : public ICam {
  public:
    BaseCam(const std::string name, const TS& algoStartTime);

    std::tuple<const bool, const int64_t, cv::Mat> getFrame() override;

    void serialize(
      CapnpOutput::CamSensor::Builder& builder,
      const int idx,
      const int64_t ts,
      const cv::Mat& img
    ) const override;

    const cv::Size getFrameSize() const override { return _frameSize; }
    const double getFrameRate() const override { return _frameRate; }
    const double getHorizontalFov() const override { return _horizontalFov; }
    const double getVerticalFov() const override { return _verticalFov; }
    const double getFocalX() const override { return _fx; }
    const double getFocalY() const override { return _fy; }
    const double getPrincipalPointX() const override { return _cx; }
    const double getPrincipalPointY() const override { return _cy; }
    const std::string getName() const override { return _name; }

  protected:
    // From width, height (in [px]) and horizontalFov (in [rad]), all other intrinsics
    // can be calculated. It assumes the principal point at the center of image (concentric lens)
    void setCamIntrinsics(const int width, const int height, const double horizontalFov);

    const std::string _name;
    double _frameRate;
    double _horizontalFov; // in [rad]
    double _verticalFov; // in [rad]
    double _fx; // focal length x in [px]
    double _fy; // focal length y in [px]
    double _cx; // principal point x in [px]
    double _cy; // principal point y in [px] 
    cv::Size _frameSize;

    cv::Mat _bufferFrame;
    cv::Mat _currFrame;
    int64_t _currTs;
    bool _validFrame;

    const TS& _algoStartTime;

    std::mutex _readMutex;
  };
}
