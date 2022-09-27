#include "video_cam.h"
#include <fstream>
#include <iostream>
#include <chrono>
#include "util/json.hpp"


data::VideoCam::VideoCam(
  const std::string name,
  const std::string& filePath
) :
  _name(name),
  _currFrameNr(0),
  _filePath(filePath)
{
  _stream.open(_filePath, cv::CAP_FFMPEG);
  if (!_stream.isOpened()) {
    throw std::runtime_error("VideoCam could not open file: " + _filePath);
  }

  _frameSize = cv::Size(_stream.get(cv::CAP_PROP_FRAME_WIDTH), _stream.get(cv::CAP_PROP_FRAME_HEIGHT));
  _frameRate = _stream.get(cv::CAP_PROP_FPS);
  const double frameCount = _stream.get(cv::CAP_PROP_FRAME_COUNT);
  _recLength = static_cast<int64>(((frameCount - 1) / _frameRate) * 1000000);
}

void data::VideoCam::fillCamData(proto::CamSensor& camSensor) {
  const int currFrameNr = _stream.get(cv::CAP_PROP_POS_FRAMES);
  const bool success = _stream.read(_currFrame);
  if (success) {
    const double tsMsec = _stream.get(cv::CAP_PROP_POS_MSEC);
    _currTs = static_cast<int64>(tsMsec * 1000.0);
    _currFrameNr = currFrameNr;

    camSensor.set_timestamp(_currTs);
    auto img = camSensor.mutable_img();
    img->set_width(_currFrame.size().width);
    img->set_height(_currFrame.size().height);
    img->set_channels(_currFrame.channels());
    img->set_data(_currFrame.data, _currFrame.size().width * _currFrame.size().height * _currFrame.channels() * sizeof(uchar));
  }
  camSensor.set_isvalid(success);
}
