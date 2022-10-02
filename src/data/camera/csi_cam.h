#pragma once
#include "icam.h"
#include "opencv2/opencv.hpp"
#include <iostream>
#include "util/cam.h"
#include "util/img.h"


namespace data {
  class CsiCam : public ICam {
  public:
    CsiCam(
      std::string camName,
      int captureWidth,
      int captureHeight,
      int horizontalFov,
      double frameRate,
      int flipMethod
    );

    void fillCamData(proto::CamSensor& camSensor, const util::TS& appStartTime) override;
    std::string getName() const override { return _name; };

  private:
    std::string _name;
    int64_t _currTs;
    cv::VideoCapture _capture;
    cv::Mat _currFrame;
    util::Cam _cam;
    util::img::Roi _roi;
  };
}