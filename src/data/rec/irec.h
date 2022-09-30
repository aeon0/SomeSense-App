#pragma once

#include <tuple>
#include <chrono>
#include "frame.pb.h"
#include "util/time.h"


namespace data {
  class IRec {
  public:
    // Fill protobuf with the frame data
    virtual void fillFrame(proto::Frame& frame) = 0;
    virtual int64_t getRecLength() const = 0;
    virtual void reset() = 0;
    virtual void setRelTs(int64_t newRelTs) = 0;
  };
}
