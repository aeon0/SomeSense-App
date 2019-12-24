#pragma once

#include <tuple>
#include "icam.h"


namespace data_reader {
  class VideoCam : public ICam {
  public:
    VideoCam(const std::string& filename);
    std::tuple<const int64, cv::Mat> getFrame() override;
    const double getFrameRate() const override { return _frameRate; }

  private:
    cv::VideoCapture _stream;
    const std::string _filename;
    double _frameRate;
  };
}
