#pragma once

#include <tuple>
#include "base_cam.h"


namespace data_reader {
  class UsbCam : public BaseCam {
  public:
    UsbCam(const std::string name, const TS& algoStartTime, const int deviceIdx);

  private:
    void readData();

    cv::VideoCapture _cam;
    int _deviceIdx;
  };
}
