#pragma once

#include <tuple>
#include "icam.h"


namespace data_reader {
  class VideoCam : public ICam {
  public:
    VideoCam(const std::string& filename, const std::string name, const std::vector<int64> timestamps = {});

    // Getters for frame data
    std::tuple<const bool, const int64, cv::Mat> getNewFrame(
      const std::chrono::time_point<std::chrono::high_resolution_clock>& algoStartTime,
      const int64 currentAlgoTs,
      const bool updateToAlgoTs) override;
    std::tuple<const bool, const int64, cv::Mat> getFrame() override;

    // Getter for additional info the the camera
    const double getFrameRate() const override { return _frameRate; }
    const cv::Size getFrameSize() const override { return cv::Size(_stream.get(cv::CAP_PROP_FRAME_WIDTH), _stream.get(cv::CAP_PROP_FRAME_HEIGHT)); }
    const std::string getName() const override { return _name; }

    // Recording specific getters (have default implementation otherwise)
    const bool isRecording() const override { return true; }
    const int64 getRecLength() const override { return _recLength; }

  private:
    const std::string _filename;
    const std::string _name;
    const std::vector<int64> _timestamps;

    cv::VideoCapture _stream;
    double _frameRate;
    int64 _recLength; // length of recording in [us]

    cv::Mat _currFrame;
    int64 _currTs;
    bool _validFrame;
  };
}
