#pragma once
#include <string>

namespace semseg {
  const int OFFSET_BOTTOM = 120; // Offset from original size (640x380)

  const std::string PATH_EDGETPU_MODEL = "/home/jo/git/app-frame/assets/od_model/semseg_quant_edgetpu.tflite";
  const std::string PATH_TFLITE_MODEL = "/home/jo/git/app-frame/assets/od_model/semseg_quant.tflite";

  const std::array<cv::Vec3b, 5> CLASS_MAPPING_COLORS = {
    cv::Vec3b( 32,  10,   0), // road
    cv::Vec3b(  0,   0, 135), // lane_markins
    cv::Vec3b(  0,   0,   0), // undriveable
    cv::Vec3b( 14,  55,   0), // movable
    cv::Vec3b( 69,   0,  69), // ego_car
  };
}