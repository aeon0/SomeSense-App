#pragma once

#include <tuple>
#include "base_cam.h"


namespace data_reader {
  class CsiCam : public BaseCam {
  public:
    CsiCam(const std::string name, const TS& algoStartTime, int captureWidth, int captureHeight, double frameRate, int flipMethod);

  private:
    void readData();

    cv::VideoCapture _cam;
    int _deviceIdx;
  };
}
