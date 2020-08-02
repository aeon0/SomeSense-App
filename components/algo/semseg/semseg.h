#pragma once

#include <iostream>
#include "opencv2/opencv.hpp"
#include "frame/runtime_meas_service.h"


namespace semseg {
  class Semseg {
  public:
    Semseg(frame::RuntimeMeasService& runtimeMeasService);
    void reset();
    void processImg(const cv::Mat &img);

  private:
    frame::RuntimeMeasService& _runtimeMeasService;
  };
}
