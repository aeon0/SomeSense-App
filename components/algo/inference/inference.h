#pragma once

#include "opencv2/opencv.hpp"
#include "frame/runtime_meas_service.h"
#include "data_reader/cams/icam.h"
#include "utilities/img.h"
#include "serialize/frame.capnp.h"
#include "params.h"
#include "tensorflow/lite/interpreter.h"


namespace inference {
  class Inference {
  public:
    Inference(frame::RuntimeMeasService& runtimeMeasService);
    void reset();
    void processImg(const cv::Mat &img);
    void serialize(CapnpOutput::CamSensor::Builder& builder);

    const cv::Mat& getSemseg() { return _semsegOut; }
    const cv::Mat& getDepth() { return _depthOut; }
    const std::vector<Object2D>& getObjects2D() { return _objects2D; }
    const util::img::Roi& getRoi() { return _roi; }

  private:
    frame::RuntimeMeasService& _runtimeMeasService;
    // TFLite and EdgeTpu
    std::unique_ptr<tflite::Interpreter> _interpreter;
    bool _edgeTpuAvailable;
    // Output data
    cv::Mat _semsegOut;
    cv::Mat _semsegImg;
    cv::Mat _depthOut;
    cv::Mat _depthImg;
    std::vector<Object2D> _objects2D;
    util::img::Roi _roi;
  };
}
