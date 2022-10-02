#pragma once

#include <tuple>
#include "opencv2/opencv.hpp"
#include "icam.h"
#include "util/cam.h"
#include "util/img.h"


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
    int64_t _currTs;
    cv::VideoCapture _capture;
    std::string _name;
    cv::Mat _currFrame;

    util::Cam _cam;
    util::img::Roi _roi;
  };
}
