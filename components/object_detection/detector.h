#pragma once

#include <iostream>
#include "opencv2/opencv.hpp"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/optional_debug_tools.h"


namespace object_detection {
  class Detector {
  public:
    void loadModel(const char* modelPath, const std::string& boxConfigPath);
    void detect(const cv::Mat& img);
  private:
    std::unique_ptr<tflite::Interpreter> _interpreter;
    std::unique_ptr<tflite::FlatBufferModel> _model;
    bool _isWarmedUp;

    int _numClasses;
    // In form of [..classes, ..offsets, ...] starting at lowest feature map and row by row (height, width)
    // offset in form of [cx, cy, width, height]
    std::vector<float> _priorBoxes;
  };
}
