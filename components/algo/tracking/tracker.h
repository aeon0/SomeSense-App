#pragma once

#include <iostream>
#include "opencv2/opencv.hpp"
#include "frame/runtime_meas_service.h"
#include "serialize/frame.capnp.h"
// #include "algo/optical_flow/optical_flow.h"


namespace tracking {
  class Tracker {
  public:
    Tracker(frame::RuntimeMeasService& runtimeMeasService);
    void reset();
    void update();

    void serialize(CapnpOutput::Frame::Builder& builder);
  private:
    frame::RuntimeMeasService& _runtimeMeasService;
  };
}
