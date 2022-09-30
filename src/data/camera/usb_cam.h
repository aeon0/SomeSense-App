#pragma once

#include <tuple>
#include "opencv2/opencv.hpp"
#include "icam.h"


namespace data {
  class UsbCam : public ICam {
  public:
    UsbCam(
      const std::string name,
      const int deviceIdx,
      const int captureWidth,
      const int captureHeight,
      const double horizontalFov
    );
    void fillCamData(proto::CamSensor& camSensor, const util::TS& appStartTime) override;
    std::string getName() const override { return _name; }

  private:
    void readData();

    double _frameRate;
    cv::Size _frameSize;
    int _horizontalFov;
    int64_t _currTs;
    cv::VideoCapture _cam;
    std::string _name;
    int _deviceIdx;
    cv::Mat _currFrame;
  };
}
