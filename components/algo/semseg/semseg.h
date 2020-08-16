#pragma once

#include "opencv2/opencv.hpp"
#include "frame/runtime_meas_service.h"
// Tensorflow Lite and EdgeTpu includes
#include "edgetpu.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"


namespace semseg {
  class Semseg {
  public:
    Semseg(frame::RuntimeMeasService& runtimeMeasService);
    void reset();
    void processImg(const cv::Mat &img);

  private:
    frame::RuntimeMeasService& _runtimeMeasService;

    std::unique_ptr<tflite::FlatBufferModel> model;
    std::shared_ptr<edgetpu::EdgeTpuContext> edgeTpuContext;
    tflite::ops::builtin::BuiltinOpResolver resolver;
    bool useTpu;

    const int INPUT_WIDTH = 320;
    const int INPUT_HEIGHT = 130;
    const int OFFSET_TOP = 60; // Offset from original size (640x380)

    const int MASK_WIDTH = 320;
    const int MASK_HEIGHT = 130;
    const std::array<cv::Vec3b, 5> CLASS_MAPPING = {
      cv::Vec3b( 32,  32,  64), // ("road", 0x202040)
      cv::Vec3b(  0,   0, 255), // ("lane_markings", 0x0000ff)
      cv::Vec3b( 96, 128, 128), // ("undriveable", 0x608080)
      cv::Vec3b(102, 255,   0), // ("movable", 0x66ff00)
      cv::Vec3b(255,   0, 204), // ("ego_car", 0xff00cc)
    };
  };
}
