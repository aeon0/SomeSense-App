#pragma once

#include <tuple>
#include <chrono>
#include "frame.pb.h"


namespace data {
  class IRec {
  public:
    // Fill protobuf with the frame data
    virtual void fillFrame(proto::Frame& frame) = 0;
    virtual void reset() = 0;
  };
}
