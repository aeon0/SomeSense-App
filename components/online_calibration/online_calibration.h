#pragma once

#include <iostream>
#include "opencv2/opencv.hpp"
#include "frame/runtime_meas_service.h"


namespace online_calibration {
  class Calibrator {
  public:
    Calibrator(frame::RuntimeMeasService& runtimeMeasService);
    void reset();
    void calibrate(const cv::Mat &img);

  private:
    cv::Mat _prevImg;
    std::vector<cv::Point2f> _featuresLastFrame;
    frame::RuntimeMeasService& _runtimeMeasService;
  };
}
