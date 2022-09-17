#pragma once

#include "opencv2/opencv.hpp"
#include "frame/runtime_meas_service.h"
#include "data_reader/cams/icam.h"
#include "utilities/img.h"
#include "serialize/frame.capnp.h"
#include "params.h"
// Tensorflow Lite and EdgeTpu includes
#include "edgetpu.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"


namespace inference {
  class Inference {
  public:
    Inference(frame::RuntimeMeasService& runtimeMeasService);
    void reset();
    void processImg(const cv::Mat &img);
    void serialize(CapnpOutput::CamSensor::Builder& builder);

    const cv::Mat& getSemseg() { return _semsegOut; }
    const cv::Mat& getDepth() { return _depthOut; }
    const util::img::Roi& getRoi() { return _roi; }

  private:
    frame::RuntimeMeasService& _runtimeMeasService;
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
    util::img::Roi _roi; // Roi regarding the output data (_semsegOut, _semsegImg, _depthOut, _depthImg)
  };
}
