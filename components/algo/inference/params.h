#pragma once
#include <string>

namespace inference {
  const int OFFSET_BOTTOM = 120; // Offset from original size (640x380)

  const std::string PATH_EDGETPU_MODEL = "./assets/od_model/multitask_edgetpu.tflite";
  const std::string PATH_TFLITE_MODEL = "./assets/od_model/multitask.tflite";
  const float QUANT_SCALE = 0.00392158; // TODO: This should also be accessable somehow when loading the model (at least it is when using python)

  const int OD_HEATMAP_IDX = 0;
  const int OD_CLASS_START_IDX = 1;
  const int SEMSEG_START_IDX = 10;
  const int DEPTH_IDX = 15;

  const float SEMSEG_THRESH = 253;
  enum SemsegCls {
    ROAD,
    LANE_MARKINGS,
    UNDRIVEABLE,
    MOVABLE,
    EGO_CAR,
    NUM_SEMSEG_CLS
  };
  const std::array<cv::Vec3b, 5> COLORS_SEMSEG = {
    cv::Vec3b( 96,  32,   0), // road, dark red
    cv::Vec3b(  0,   0, 255), // lane_markins, red
    cv::Vec3b(  0,   0,   0), // undriveable, green/brown
    cv::Vec3b(102, 255,   0), // movable, green
    cv::Vec3b(255,   0, 204), // ego_car, purple
  };

  enum OdCls {
    CAR,
    TRUCK,
    VAN,
    MOTORBIKE,
    CYCLIST,
    PED,
    NUM_OD_CLS
  };
  const std::array<cv::Vec3b, 6> COLORS_OD = {
    cv::Vec3b( 96,  96, 192), // CAR: red
    cv::Vec3b( 96, 192, 192), // TRUCK: yellow
    cv::Vec3b(128, 192,  96), // VAN: turquoise
    cv::Vec3b(194,  96,  64), // MOTORBIKE: blue
    cv::Vec3b(255,   0, 204), // CYCLIST: green
    cv::Vec3b(196,  64, 196), // PED: pink
  };
}
