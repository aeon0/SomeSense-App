#pragma once

#include "NvInferRuntimeCommon.h"
#include <iostream>


namespace object_detection {
  class TRTLogger: public nvinfer1::ILogger {
  public:
    void log(Severity severity, const char* msg) override {
      std::cout << "[TRT]: " << msg << std::endl;
    }

    nvinfer1::ILogger& getTRTLogger()
    {
      return *this;
    }
  };
}
