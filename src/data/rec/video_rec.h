#pragma once

#include <atomic>
#include <tuple>
#include "irec.h"
#include "opencv2/opencv.hpp"
#include "frame.pb.h"


namespace data {
  class VideoRec : public IRec {
  public:
    VideoRec(const std::string& filePath);
    void fillFrame(proto::Frame& frame) override;
    void reset() override;

  private:
    cv::VideoCapture _stream;
    std::string _filePath;

    int64_t _recLength; // length of recording in [us]
    double _frameRate;
    cv::Size _frameSize;
    int _currFrameNr;
    cv::Mat _currFrame;
    int64_t _currTs;

    std::mutex _readLock;
  };
}
