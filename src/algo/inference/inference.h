#pragma once

#include "opencv2/opencv.hpp"
#include "util/runtime_meas_service.h"
#include "util/img.h"
#include "params.h"
#include "frame.pb.h"
// Tensorflow Lite and EdgeTpu includes
#include "edgetpu.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"


namespace algo {
  class Inference {
  public:
    Inference(util::RuntimeMeasService& runtimeMeasService);
    void reset();
    void processImg(const cv::Mat &img);
    void serialize(proto::CamSensor& camSensor);

    const cv::Mat& getSemseg() { return _semsegOut; }
    const cv::Mat& getDepth() { return _depthOut; }
  private:
    util::RuntimeMeasService& _runtimeMeasService;
    // TFLite and EdgeTpu
    std::unique_ptr<tflite::FlatBufferModel> _model;
    std::shared_ptr<edgetpu::EdgeTpuContext> _edgeTpuContext;
    tflite::ops::builtin::BuiltinOpResolver _resolver;
    std::unique_ptr<tflite::Interpreter> _interpreter;
    bool _edgeTpuAvailable;
    // Output data
    cv::Mat _semsegOut;
    cv::Mat _semsegImg;
    cv::Mat _depthOut;
    cv::Mat _depthImg;
    util::img::Roi _roi;
  };
}
