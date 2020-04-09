#pragma once

#include <iostream>
#include "opencv2/opencv.hpp"
#include "frame/runtime_meas_service.h"


namespace online_calibration {
  class Calibrator {
  public:
    Calibrator(frame::RuntimeMeasService& runtimeMeasService);
    void reset();

  private:
    frame::RuntimeMeasService& _runtimeMeasService;
  };
}
