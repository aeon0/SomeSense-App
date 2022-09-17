#pragma once

#include <iostream>
#include "opencv2/opencv.hpp"
#include "frame/runtime_meas_service.h"
#include "serialize/frame.capnp.h"


namespace optical_flow {
  class OpticalFlow {
  public:
    typedef std::vector<std::pair<cv::Point2f, cv::Point2f>> FlowVector; // pair has Points at [t-1, t]

    OpticalFlow(frame::RuntimeMeasService& runtimeMeasService);
    void reset();
    void processImg(const cv::Mat &img, const int64_t ts);
    void serialize(CapnpOutput::CamSensor::OpticalFlow::Builder& builder);
    const std::pair<FlowVector, double> getFlow() const { return std::make_pair(_flow, _deltaTime); };

  private:
    frame::RuntimeMeasService& _runtimeMeasService;

    int64_t _prevTs;
    cv::Mat _prevImg;
    FlowVector _flow;
    double _deltaTime; // delta time of the flow measurements in [ms]
  };
}
