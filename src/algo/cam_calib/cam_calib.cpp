#include "cam_calib.h"
#include <fstream>
#include <cmath>
#include <iostream>
#include "algo/inference/params.h"
#include "algo/cam_calib/params.h"
#include "util/json.hpp"


algo::CamCalib::CamCalib()
{
  reset();
  std::ifstream ifs(MANUAL_CALIB_PATH);
  if (ifs.good()) {
    std::cout << "Loading Calibration from Path: " << MANUAL_CALIB_PATH << std::endl;
    auto jd = nlohmann::json::parse(ifs);
    _staticCam.setExtrinsics(jd["x"], jd["y"], jd["z"], jd["pitch"], jd["roll"], jd["yaw"]);
  }
}

void algo::CamCalib::reset() {
}

void algo::CamCalib::run(const proto::CamSensor* camSensor) {
  // Currently no clibration implemented;
  _staticCam.setIntrinsics(camSensor->calib());
}

void algo::CamCalib::serialize(proto::CamCalibration* calib) {
  _staticCam.fillProtoCalib(calib);
}
