#pragma once

#include <iostream>
#include <memory>
#include "NvInfer.h"
#include "NvOnnxConfig.h"
#include "NvOnnxParser.h"
#include "opencv2/opencv.hpp"
#include "trt_logger.h"


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
    void loadModel(const std::string& modelPath, const std::string& boxConfigPath);
    void detect(const cv::Mat& img);

  private:
    void loadModelFromOnnx(const std::string& modelPath);

    int _numClasses;
    // In form of [..classes, ..offsets, ...] starting at lowest feature map and row by row (height, width)
    // offset in form of [cx, cy, width, height]
    std::vector<float> _priorBoxes;
    TRTLogger _logger;

    std::shared_ptr<nvinfer1::ICudaEngine> _engine; // The TensorRT engine used to run the network
    float* _inPtr = NULL;
    float* _outPtr = NULL;

    nvinfer1::Dims _inputDim;
    nvinfer1::Dims _outputDim;
  };
}
