#pragma once

#include "opencv2/opencv.hpp"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/optional_debug_tools.h"


namespace object_detection {
  class Detector {
  public:
    void loadModel(const char* path);
    void detect(const cv::Mat& img);
  private:
    std::unique_ptr<tflite::Interpreter> _interpreter;
    std::unique_ptr<tflite::FlatBufferModel> _model;
    bool _isWarmedUp;
  };
}
