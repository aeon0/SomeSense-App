#pragma once

#include <tuple>
#include "base_cam.h"


namespace data_reader {
  class VideoCam : public BaseCam {
  public:
    VideoCam( const std::string name, const std::string& filename, const std::vector<int64> timestamps = {});

    // Getters for frame data
    std::tuple<const bool, const int64, cv::Mat> getNewFrame(
      const std::chrono::time_point<std::chrono::high_resolution_clock>& algoStartTime,
      const int64 currentAlgoTs,
      const bool updateToAlgoTs) override;

    // Recording specific getters (have default implementation otherwise)
    const bool isRecording() const override { return true; }
    const int64 getRecLength() const override { return _recLength; }

  private:
    const std::string _filename;
    const std::vector<int64> _timestamps;

    cv::VideoCapture _stream;
    int64 _recLength; // length of recording in [us]
  };
}
