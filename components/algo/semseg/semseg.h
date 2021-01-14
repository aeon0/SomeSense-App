#pragma once

#include "opencv2/opencv.hpp"
#include "frame/runtime_meas_service.h"
#include "data_reader/cams/icam.h"
#include "utilities/img.h"
#include "serialize/frame.capnp.h"
// Tensorflow Lite and EdgeTpu includes
#include "edgetpu.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"


namespace semseg {
  class Semseg {
  public:
    struct DriveableBin {
      cv::Point3f startPos; // start position of bin in [m] (autosar)
      float extendX; // extend of bin in x-dir for startPos
      float absExtendY; // extend of bin evenly in +y and -y dir evenly e.g. for extend 4 it extends +2 and -2 from startPos in y dir
    };

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
    util::img::Roi _maskRoi;
    std::vector<cv::Point3f> _obstacles; // x, y, z in autosar coordinate system of movable objects
    std::vector<cv::Point3f> _laneMarkings; // x, y, z in autosar coordinate system of lane markings
    std::vector<DriveableBin> _driveBins; // drivable bins for each mask column
  };
}
