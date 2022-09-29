#include "usb_cam.h"
#include <iostream>
#include <thread>


data::UsbCam::UsbCam(
  const std::string name,
  const TS algoStartTime,
  const int deviceIdx,
  const int captureWidth,
  const int captureHeight,
  const double horizontalFov
):
  _name(name),
  _algoStartTime(algoStartTime),
  _deviceIdx(deviceIdx),
  _horizontalFov(horizontalFov)
{
  _cam.open(_deviceIdx, cv::CAP_V4L);
  if (!_cam.isOpened()) {
    throw std::runtime_error("Could not open USB Camera at index: " + std::to_string(_deviceIdx));
  }

  _cam.set(cv::CAP_PROP_FRAME_WIDTH, captureWidth);
  _cam.set(cv::CAP_PROP_FRAME_HEIGHT, captureHeight);
  _frameRate = _cam.get(cv::CAP_PROP_FPS);
  _frameSize = cv::Size(_cam.get(cv::CAP_PROP_FRAME_WIDTH), _cam.get(cv::CAP_PROP_FRAME_HEIGHT));
}

void data::UsbCam::fillCamData(proto::CamSensor& camSensor) {
  const auto captureTime = std::chrono::high_resolution_clock::now();
  bool success = _cam.read(_currFrame);
  if (success) {
    _currTs = static_cast<int64_t>(std::chrono::duration<double, std::micro>(captureTime - _algoStartTime).count());
    camSensor.set_timestamp(_currTs);
    auto img = camSensor.mutable_img();
    img->set_width(_currFrame.size().width);
    img->set_height(_currFrame.size().height);
    img->set_channels(_currFrame.channels());
    img->set_data(_currFrame.data, _currFrame.size().width * _currFrame.size().height * _currFrame.channels() * sizeof(uchar));
    // TODO: Set intrinsics and extrinsics here
    // setIntrinsics(_cam.get(cv::CAP_PROP_FRAME_WIDTH), _cam.get(cv::CAP_PROP_FRAME_HEIGHT), horizontalFov);
    // setExtrinsics(0.0, 0.0, 1.7, 0.0, 0.0, 0.0);
  }
  camSensor.set_isvalid(success);
}
