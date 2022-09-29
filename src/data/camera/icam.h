#pragma once

#include <tuple>
#include <chrono>
#include "frame.pb.h"


namespace data {
  class ICam {
  public:
    // Fill protobuf with the frame data
    virtual void fillCamData(proto::CamSensor& camSensor) = 0;

    // Return a name for the sensor (used for storing)
    virtual std::string getName() const = 0;

    // Return if the camera is based on video data
    virtual bool isRecording() const { return false; }
    virtual int64_t getRecLength() const  { return -1; }
  };
}
