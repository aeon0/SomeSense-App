#pragma once

#include "opencv2/opencv.hpp"
#include "util/runtime_meas_service.h"
#include "util/cam.h"
#include "util/img.h"
#include "frame.pb.h"


namespace algo {
  class Pointcloud {
  public:
    Pointcloud(util::RuntimeMeasService& runtimeMeasService);
    void reset();
    void run(const proto::Frame& frame);
    void serialize(proto::Frame& frame);

  private:
    util::RuntimeMeasService& _runtimeMeasService;
    // Memory alloc for input data the proto Frame is converted to
    cv::Mat _semseg;
    cv::Mat _depth;
    util::img::Roi _roi;
    util::Cam _cam;

    // Results
    std::vector<cv::Point3f> _obstacles; // x, y, z in autosar coordinate system of movable objects
    std::vector<cv::Point3f> _laneMarkings; // x, y, z in autosar coordinate system of lane markings
  };
}
