#pragma once

#include <tuple>
#include "base_cam.h"


namespace data_reader {
  class VideoCam : public BaseCam {
  public:
    VideoCam( const std::string name, const TS& algoStartTime, const std::string& filename, const std::vector<int64> timestamps = {});

    // Recording specific getters (have default implementation otherwise)
    const bool isRecording() const override { return true; }
    const int64_t getRecLength() const override { return _recLength; }

  private:
    const std::string _filename;
    const std::vector<int64> _timestamps;

    cv::VideoCapture _stream;
    int64_t _recLength; // length of recording in [us]
  };
}
