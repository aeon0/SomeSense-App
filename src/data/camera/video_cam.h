#pragma once

#include <atomic>
#include <tuple>
#include "icam.h"
#include "opencv2/opencv.hpp"
#include "frame.pb.h"


namespace data {
  class VideoCam : public ICam {
  public:
    VideoCam(const std::string name, const std::string& filePath);

    void fillCamData(proto::CamSensor& camSensor) override;
    virtual std::string getName() const override { return _name; }
    bool isRecording() const override { return true; }

  private:
    cv::VideoCapture _stream;
    std::string _name;
    std::string _filePath;

    int64_t _recLength; // length of recording in [us]
    double _frameRate;
    cv::Size _frameSize;
    int _currFrameNr;
    cv::Mat _currFrame;
    int64_t _currTs;
  };
}
