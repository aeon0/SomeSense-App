#pragma once

#include "opencv2/opencv.hpp"
#include "frame/runtime_meas_service.h"
#include "data_reader/cams/icam.h"
#include "serialize/frame.capnp.h"
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
    void processImg(const cv::Mat &img, const data_reader::ICam &cam);
    void serialize(CapnpOutput::CamSensor::Semseg::Builder& builder);

  private:
    frame::RuntimeMeasService& _runtimeMeasService;
    // TFLite and EdgeTpu
    std::unique_ptr<tflite::FlatBufferModel> _model;
    std::shared_ptr<edgetpu::EdgeTpuContext> _edgeTpuContext;
    tflite::ops::builtin::BuiltinOpResolver _resolver;
    bool _edgeTpuAvailable;
    // Output data
    cv::Mat _semsegMask;
    std::vector<cv::Point3f> _pointCloud; // x, y, z in autosar coordinate system
  };
}
