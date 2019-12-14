#pragma once

#include <iostream>
#include <memory>
#include "opencv2/opencv.hpp"
#include "logger.h"


namespace object_detection {
  struct InferDeleter {
    template <typename T>
    void operator()(T* obj) const {
      if (obj) {
        obj->destroy();
      }
    }
  };

  template <typename T>
  using TRTUniquePtr = std::unique_ptr<T, InferDeleter>;

  class Detector {
  public:
    void loadModel(const char* modelPath, const std::string& boxConfigPath);
    void detect(const cv::Mat& img);

  private:
    int _numClasses;
    // In form of [..classes, ..offsets, ...] starting at lowest feature map and row by row (height, width)
    // offset in form of [cx, cy, width, height]
    std::vector<float> _priorBoxes;
    TRTLogger _logger;
  };
}
