#pragma once

#include <tuple>
#include <chrono>
#include "opencv2/opencv.hpp"


namespace data_reader {
  class ICam {
  public:
    // Get previous frame and returns {success, timestamp in [us], image}
    virtual std::tuple<const bool, const int64_t, cv::Mat> getFrame() = 0;

    // Return the base (optimal) fps possible for this sensor
    virtual const double getFrameRate() const = 0;

    // Return frame size
    virtual const cv::Size getFrameSize() const = 0;

    // Return a name for the sensor (used for storing)
    virtual const std::string getName() const = 0;

    // Return if the camera is based on video data
    virtual const bool isRecording() const { return false; }

    // Return rec length in us
    virtual const int64_t getRecLength() const { return 0; }

  };
}
