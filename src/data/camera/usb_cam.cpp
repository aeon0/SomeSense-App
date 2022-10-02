#include "usb_cam.h"
#include <iostream>
#include <thread>


data::UsbCam::UsbCam(
  const std::string name,
  const int deviceIdx,
  const int captureWidth,
  const int captureHeight,
  const double horizontalFov
):
  _name(name)
{
  _capture.open(deviceIdx, cv::CAP_V4L);
  if (!_capture.isOpened()) {
    throw std::runtime_error("Could not open USB Camera at index: " + std::to_string(deviceIdx));
  }

  _capture.set(cv::CAP_PROP_FRAME_WIDTH, captureWidth);
  _capture.set(cv::CAP_PROP_FRAME_HEIGHT, captureHeight);
  auto frameSize = cv::Size(_capture.get(cv::CAP_PROP_FRAME_WIDTH), _capture.get(cv::CAP_PROP_FRAME_HEIGHT));

  _cam = util::Cam();
  _cam.setIntrinsics(frameSize.width, frameSize.height, horizontalFov);
}

void data::UsbCam::fillCamData(proto::CamSensor& camSensor, const util::TS& appStartTime) {
  const auto captureTime = std::chrono::high_resolution_clock::now();
  bool success = _capture.read(_currFrame);
  if (success) {
    camSensor.set_absts(util::timepointToInt64(captureTime));
    camSensor.set_relts(util::calcDurationInInt64(captureTime, appStartTime));

    auto img = camSensor.mutable_img();
    img->set_width(_currFrame.size().width);
    img->set_height(_currFrame.size().height);
    img->set_channels(_currFrame.channels());
    img->set_data(_currFrame.data, _currFrame.size().width * _currFrame.size().height * _currFrame.channels() * sizeof(uchar));

    // set intrinsics
    _cam.fillProtoCalib(camSensor.mutable_calib());
  }
  camSensor.set_isvalid(success);
}
