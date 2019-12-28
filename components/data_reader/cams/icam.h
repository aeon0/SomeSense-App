#pragma once

#include <tuple>
#include <chrono>
#include "opencv2/opencv.hpp"

namespace data_reader {
  class ICam {
  public:
    // Get next frame and returns {success, timestamp in [us], image}
    // The currentAlgoTs and updateToAlgoTs are only relevant for recorded data
    virtual std::tuple<const bool, const int64, cv::Mat> getNewFrame(
      const std::chrono::time_point<std::chrono::high_resolution_clock>& algoStartTime,
      const int64 currentAlgoTs,
      const bool updateToAlgoTs) = 0;

    // Get previous frame and returns {success, timestamp in [us], image}
    virtual std::tuple<const bool, const int64, cv::Mat> getFrame() = 0;

    // Return the base (optimal) fps possible for this sensor
    virtual const double getFrameRate() const = 0;

    // Return frame size
    virtual const cv::Size getFrameSize() const = 0;

    // Return a name for the sensor (used for storing)
    virtual const std::string getName() const = 0;

    // Return if the camera is based on video data
    virtual const bool isRecording() const { return false; }

    // Return rec length in us
    virtual const int64 getRecLength() const { return 0; }

  };
}
