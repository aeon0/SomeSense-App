#pragma once

#include <tuple>
#include <chrono>
#include "frame.pb.h"
#include "util/time.h"


namespace data {
  class ICam {
  public:
    // Fill protobuf with the frame data
    virtual void fillCamData(proto::CamSensor& camSensor, const util::TS& appStartTime) = 0;

    // Return a name for the sensor (used for storing)
    virtual std::string getName() const = 0;
  };
}
