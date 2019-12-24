#pragma once

#include "icam.h"

namespace data_reader {
  class VideoCam : public ICam {
  public:
    VideoCam(const std::string& filename);
    cv::Mat getFrame();

    const double getFrameRate() const { return _frameRate; }

  private:
    cv::VideoCapture _stream;
    std::string _filename;

    double _frameRate;
  };
}
