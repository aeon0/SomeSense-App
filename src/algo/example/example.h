#pragma once

#include <iostream>
#include "opencv2/opencv.hpp"
#include "frame/runtime_meas_service.h"


namespace some_algo {
  class Example {
  public:
    Example(frame::RuntimeMeasService& runtimeMeasService);
    void reset();
    void doStuff();

  private:
    frame::RuntimeMeasService& _runtimeMeasService;
  };
}
