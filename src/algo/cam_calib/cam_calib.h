#pragma once

#include "opencv2/opencv.hpp"
#include "util/cam.h"
#include "frame.pb.h"


namespace algo {
  class CamCalib {
  public:
    CamCalib();
    void reset();
    void run(const proto::CamSensor& camSensor);
    void serialize(proto::CamCalibration* calib);

  private:
    util::Cam _dynCam;
    util::Cam _staticCam;
  };
}
