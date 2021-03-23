#pragma once

#include "opencv2/opencv.hpp"
#include "frame/runtime_meas_service.h"
#include "serialize/frame.capnp.h"
#include "data_reader/cams/icam.h"
#include "utilities/img.h"


namespace cam_calib {
  class CamCalib {
  public:
    CamCalib(frame::RuntimeMeasService& runtimeMeasService, data_reader::ICam& cam);
    void reset();
    void calibrate(const cv::Mat& semseg, const cv::Mat& depth, const util::img::Roi& roi);

  private:
    frame::RuntimeMeasService& _runtimeMeasService;
    data_reader::ICam& _cam;
    float _pitch;
    float _tz;
  };
}
