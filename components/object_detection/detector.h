#pragma once

#include <iostream>
#include "opencv2/opencv.hpp"



namespace object_detection {
  class Detector {
  public:
    void loadModel(const char* modelPath, const std::string& boxConfigPath);
    void detect(const cv::Mat& img);

  private:
    int _numClasses;
    // In form of [..classes, ..offsets, ...] starting at lowest feature map and row by row (height, width)
    // offset in form of [cx, cy, width, height]
    std::vector<float> _priorBoxes;
  };
}
