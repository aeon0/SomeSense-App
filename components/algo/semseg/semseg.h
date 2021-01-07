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

    std::unique_ptr<tflite::FlatBufferModel> _model;
    std::shared_ptr<edgetpu::EdgeTpuContext> _edgeTpuContext;
    tflite::ops::builtin::BuiltinOpResolver _resolver;
    bool _edgeTpuAvailable;
  };
}
