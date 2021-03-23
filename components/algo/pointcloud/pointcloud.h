#pragma once

#include "opencv2/opencv.hpp"
#include "frame/runtime_meas_service.h"
#include "serialize/frame.capnp.h"
#include "utilities/img.h"
#include "data_reader/cams/icam.h"


namespace pointcloud {
  class Pointcloud {
  public:
    Pointcloud(frame::RuntimeMeasService& runtimeMeasService);
    void reset();
    void processData(const cv::Mat& semseg, const cv::Mat& depth, const util::img::Roi& roi, const data_reader::ICam& cam);
    void serialize(CapnpOutput::Frame::Builder& builder);

  private:
    frame::RuntimeMeasService& _runtimeMeasService;

    std::vector<cv::Point3f> _obstacles; // x, y, z in autosar coordinate system of movable objects
    std::vector<cv::Point3f> _laneMarkings; // x, y, z in autosar coordinate system of lane markings
  };
}
