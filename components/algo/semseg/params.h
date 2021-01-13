#pragma once
#include <string>

namespace semseg {
  const int OFFSET_BOTTOM = 120; // Offset from original size (640x380)

  const std::string PATH_EDGETPU_MODEL = "./assets/od_model/semseg_quant_edgetpu.tflite";
  const std::string PATH_TFLITE_MODEL = "./assets/od_model/semseg_quant.tflite";

  enum {
    ROAD,
    LANE_MARKINGS,
    UNDRIVEABLE,
    MOVABLE,
    EGO_CAR
  };
  const std::array<cv::Vec3b, 5> CLASS_MAPPING_COLORS = {
    cv::Vec3b( 32,  32,  64), // road, dark red
    cv::Vec3b(  0,   0, 255), // lane_markins, red
    cv::Vec3b( 96, 128, 128), // undriveable, green/brown
    cv::Vec3b(102, 255,   0), // movable, green
    cv::Vec3b(255,   0, 204), // ego_car, purple
  };
}
