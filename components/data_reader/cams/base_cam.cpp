#include "base_cam.h"

data_reader::BaseCam::BaseCam(const std::string name, const TS& algoStartTime) : 
  _name(name), _algoStartTime(algoStartTime), _currTs(-1), _validFrame(false) {}

std::tuple<const bool, const int64_t, cv::Mat> data_reader::BaseCam::getFrame() {
  std::lock_guard<std::mutex> lockGuard(_readMutex);
  bool success = false;
  if (_currTs >= 0 && _validFrame) {
    success = true;
  }
  // TODO: maybe speed up possible with memcopy just data
  return {success, _currTs, _currFrame.clone() };
}

void data_reader::BaseCam::setCamIntrinsics(const int width, const int height, const double horizontalFov) {
  assert(horizontalFov > 0.0);
  assert(width > 0);
  assert(height > 0);

  // Set frame size
  _frameSize = cv::Size(width, height);
  // Set principal point
  _cx = static_cast<double>(_frameSize.width) / 2.0;
  _cy = static_cast<double>(_frameSize.height) / 2.0;
  // Set field of view
  _horizontalFov = horizontalFov;
  // Set focal length
  _fx = (static_cast<double>(width) * 0.5) / tan(_horizontalFov * 0.5);
  // assuming concentric lens set _fy and calc _verticalFov
  _fy = _fx;
  _verticalFov = atan((static_cast<double>(height) * 0.5) / _fy) * 2.0;
}

void data_reader::BaseCam::serialize(
  CapnpOutput::CamSensor::Builder& builder,
  const int idx,
  const int64_t ts,
  const cv::Mat& img
) const {
  // Add sensor to outputstate
  builder.setIdx(idx);
  builder.setTimestamp(ts);
  builder.setKey(getName());
  builder.setFocalLengthX(getFocalX());
  builder.setFocalLengthY(getFocalY());
  builder.setPrincipalPointX(getPrincipalPointX());
  builder.setPrincipalPointY(getPrincipalPointY());

  // TODO: fill these we calibrated intrinsics
  builder.setX(-1.5);
  builder.setY(0);
  builder.setZ(1.0);
  builder.setYaw(0.05);
  builder.setPitch(0.1);
  builder.setRoll(0.02);

  builder.setFovHorizontal(getHorizontalFov());
  builder.setFovVertical(getVerticalFov());

  // Fill img
  builder.getImg().setWidth(img.size().width);
  builder.getImg().setHeight(img.size().height);
  builder.getImg().setChannels(img.channels());
  builder.getImg().setData(
    kj::arrayPtr(img.data, img.size().width * img.size().height * img.channels() * sizeof(uchar))
  );
}
