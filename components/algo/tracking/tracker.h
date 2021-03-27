#pragma once

#include <iostream>
#include "opencv2/opencv.hpp"
#include "frame/runtime_meas_service.h"
#include "serialize/frame.capnp.h"
#include "algo/inference/params.h"
#include "data_reader/cams/icam.h"
// #include "algo/inference/inference.h"
// #include "algo/optical_flow/optical_flow.h"


namespace tracking {
  class Tracker {
  public:
    Tracker(frame::RuntimeMeasService& runtimeMeasService);
    void reset();
    void update(const std::vector<inference::Object2D>& objects2D, const data_reader::ICam& cam);

    void serialize(CapnpOutput::Frame::Builder& builder);
  private:
    struct Track {
      cv::Point3f coord;
    };

    frame::RuntimeMeasService& _runtimeMeasService;
    std::vector<Track> _tracks;
  };
}
