#pragma once

#include "opencv2/opencv.hpp"

namespace data_reader {
  class ICam {
  public:
    virtual cv::Mat getFrame() = 0;
  };
}
