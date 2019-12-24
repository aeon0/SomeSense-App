#pragma once

#include <tuple>
#include "opencv2/opencv.hpp"

namespace data_reader {
  class ICam {
  public:
    // returns timestamp in [us] + image
    virtual std::tuple<const int64, cv::Mat> getFrame() = 0;
    virtual const double getFrameRate() const = 0;
  };
}
