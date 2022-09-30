#include "video_rec.h"
#include <fstream>
#include <iostream>
#include <chrono>
#include "util/json.hpp"


data::VideoRec::VideoRec(
  const std::string& filePath
) :
  _currFrameNr(0),
  _filePath(filePath)
{
  _stream.open(_filePath, cv::CAP_FFMPEG);
  if (!_stream.isOpened()) {
    throw std::runtime_error("VideoRec could not open file: " + _filePath);
  }

  _frameSize = cv::Size(_stream.get(cv::CAP_PROP_FRAME_WIDTH), _stream.get(cv::CAP_PROP_FRAME_HEIGHT));
  _frameRate = _stream.get(cv::CAP_PROP_FPS);
  const double frameCount = _stream.get(cv::CAP_PROP_FRAME_COUNT);
  _recLength = static_cast<int64>(((frameCount - 1) / _frameRate) * 1000000);
}

void data::VideoRec::reset() {
  std::lock_guard<std::mutex> lock(_readLock);
  _currFrameNr = 0;
  _stream.set(cv::CAP_PROP_POS_FRAMES, _currFrameNr);
}

void data::VideoRec::fillFrame(proto::Frame& frame) {
  std::unique_lock<std::mutex> lock(_readLock);
  _currFrameNr = _stream.get(cv::CAP_PROP_POS_FRAMES);
  const bool success = _stream.read(_currFrame);
  _currTs = static_cast<int64_t>(_stream.get(cv::CAP_PROP_POS_MSEC) * 1000.0);
  lock.unlock();

  if (success) {
    frame.set_relts(_currTs);
    frame.set_absts(_currTs);
    frame.set_appstarttime(0);
    frame.set_isrec(true);
    frame.mutable_recdata()->set_reclength(_recLength);

    auto camSensor = frame.mutable_camsensors()->Add();
    camSensor->set_isvalid(true);
    camSensor->set_relts(_currTs);
    camSensor->set_absts(_currTs);
    auto img = camSensor->mutable_img();
    img->set_width(_currFrame.size().width);
    img->set_height(_currFrame.size().height);
    img->set_channels(_currFrame.channels());
    img->set_data(_currFrame.data, _currFrame.size().width * _currFrame.size().height * _currFrame.channels() * sizeof(uchar));
    // TODO: Set intrinsics and extrinsics here
  }
}
