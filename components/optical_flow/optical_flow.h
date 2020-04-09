#pragma once

#include <iostream>
#include "opencv2/opencv.hpp"
#include "frame/runtime_meas_service.h"


namespace optical_flow {
  class OpticalFlow {
  public:
    // pair has Points at [t-1, t]
    typedef std::vector<std::pair<cv::Point2f, cv::Point2f>> FlowVector;

    OpticalFlow(frame::RuntimeMeasService& runtimeMeasService);
    void reset();
    void update(const cv::Mat &img, const int64_t ts);

    const std::pair<FlowVector, double> getFlow() const { return std::make_pair(_flow, _deltaTime); };

  private:
    int64_t _prevTs;
    cv::Mat _prevImg;
    std::vector<cv::Point2f> _prevFreatures;
    int _framesSinceRefresh;
    frame::RuntimeMeasService& _runtimeMeasService;

    cv::Mat _fundamentalMat;
    FlowVector _flow;
    double _deltaTime; // delta time of the flow measurement in [ms]

    // After this many updates, the features are calculated again
    const int REFRESH_AFTER = 3;
  };
}
