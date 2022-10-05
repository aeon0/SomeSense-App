#pragma once
#include "icam.h"
#include "opencv2/opencv.hpp"
#include <iostream>
#include "util/cam.h"
#include "util/img.h"

/*
Some Helper functions:

# show all supported formats for the camera (might have to do: sudo apt-get install v4l-utils)
v4l2-ctl --list-formats-ext

# test your gstreamer pipline
gst-launch-1.0 v4l2src device = /dev/video0 ! video/x-raw,format=YUY2,width=640,height=480,framerate=30/1 ! videoconvert ! video/x-raw,format=BGR ! appsink drop=1
*/

namespace data {
  class CsiCam : public ICam {
  public:
    CsiCam(
      std::string camName,
      int captureWidth,
      int captureHeight,
      int horizontalFov,
      double frameRate,
      int flipMethod
    );

    void fillCamData(proto::CamSensor& camSensor, const util::TS& appStartTime) override;
    std::string getName() const override { return _name; };

  private:
    std::string _name;
    int64_t _currTs;
    cv::VideoCapture _capture;
    cv::Mat _currFrame;
    util::Cam _cam;
    util::img::Roi _roi;
  };
}