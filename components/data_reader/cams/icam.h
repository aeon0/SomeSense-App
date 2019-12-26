#pragma once

#include <tuple>
#include "opencv2/opencv.hpp"

namespace data_reader {
  class ICam {
  public:
    // returns timestamp in [us] + image
    virtual std::tuple<const int64, cv::Mat> getFrame() = 0;
    // Return the base (optimal) fps possible for this sensor
    virtual const double getFrameRate() const = 0;
    // Return if the camera is based on video data
    virtual const bool isRecording() const { return false; }
    // Return rec length in us
    virtual const int64 getRecLength() const { return 0; }
  };
}
