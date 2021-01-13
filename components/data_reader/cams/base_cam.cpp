#include "base_cam.h"


data_reader::BaseCam::BaseCam(const std::string name, const TS& algoStartTime) : 
  _name(name), _algoStartTime(algoStartTime), _currTs(-1), _validFrame(false) 
{
  // TODO: Set extrinsics with camera calibration (static and/or dynamic)
  setExtrinsics(-0.7, 0.0, 1.3, 0.065, 0.0, 0.0);
}

std::tuple<const bool, const int64_t, cv::Mat> data_reader::BaseCam::getFrame() {
  std::lock_guard<std::mutex> lockGuard(_readMutex);
  bool success = false;
  if (_currTs >= 0 && _validFrame) {
    success = true;
  }
  // TODO: maybe speed up possible with memcopy just data
  return {success, _currTs, _currFrame.clone() };
}

void data_reader::BaseCam::setExtrinsics(float tx, float ty, float tz, float pitch, float roll, float yaw)
{
  _tx = tx;
  _ty = ty;
  _tz = tz;
  _pitch = pitch;
  _roll = roll;
  _yaw = yaw;

  // roll
  cv::Mat rotX = (cv::Mat_<float>(4, 4) << 
    1,           0,          0, 0,
    0,  cos(_roll), sin(_roll), 0,
    0, -sin(_roll), cos(_roll), 0,
    0,           0,          0, 1);
  // pitch
  cv::Mat rotY = (cv::Mat_<float>(4, 4) <<
    cos(_pitch), 0, -sin(_pitch), 0,
              0, 1,            0, 0,
    sin(_pitch), 0,  cos(_pitch), 0,
              0, 0,            0, 1);
  // yaw
  cv::Mat rotZ = (cv::Mat_<float>(4, 4) <<
     cos(_yaw), sin(_yaw), 0, 0,
    -sin(_yaw), cos(_yaw), 0, 0,
             0,         0, 1, 0,
             0,         0, 0, 1);
  // translation
  cv::Mat trans = (cv::Mat_<float>(4, 4) <<
    1, 0, 0, -_tx,
    0, 1, 0, -_ty,
    0, 0, 1, -_tz,
    0, 0, 0,   1);
  // Flip axis (camera -> autosar)
  cv::Mat flipAxis = (cv::Mat_<float>(4, 4) <<
    0, -1,  0, 0,
    0,  0, -1, 0,
    1,  0,  0, 0,
    0,  0,  0, 1);

  _poseMat = flipAxis * rotX * rotY * rotZ * trans;
}

void data_reader::BaseCam::setIntrinsics(const int width, const int height, const double horizontalFov) {
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

  _camMat = (cv::Mat_<float>(4, 4) <<
    _fx,   0, _cx, 0,
      0, _fy, _cy, 0,
      0,   0,   1, 0,
      0,   0,   0, 1);
}

cv::Point3f data_reader::BaseCam::imageToWorldKnownZ(cv::Point2f imgCoord, float z) const
{
  // cv::Point2f normImgCoord = util::normalizeImgCoord(imgCoord, _fx, cv::Point2f(_cx, _cy));
  cv::Point2f normImgCoord((imgCoord.x - _cx) * (1 / _fx), (imgCoord.y - _cy) * (1 / _fy));

  // Rotate the norm cam coordinates (x, y, 1) to world coordinate system
  cv::Mat normImgMat = (cv::Mat_<float>(4, 1) << normImgCoord.x, normImgCoord.y, 1, 0);
  cv::Mat rotatedCamMat = _poseMat.t() * normImgMat;

  const float zCam = (z - _tz) * (1 / rotatedCamMat.at<float>(2));
  const float x = rotatedCamMat.at<float>(0) * zCam + _tx;
  const float y = rotatedCamMat.at<float>(1) * zCam + _ty;

  cv::Point3f result(x, y, z);
  return result;
}

cv::Point2f data_reader::BaseCam::worldToImage(cv::Point3f worldCoord) const
{
  cv::Mat worldCoordMat = (cv::Mat_<float>(4, 1) << worldCoord.x, worldCoord.y, worldCoord.z, 1);
  cv::Mat imgCoordMat = _camMat * _poseMat * worldCoordMat;
  cv::Point2f imgCoord(imgCoordMat.at<float>(0) / imgCoordMat.at<float>(2), imgCoordMat.at<float>(1) / imgCoordMat.at<float>(2));
  return imgCoord;
}

void data_reader::BaseCam::serialize(
  CapnpOutput::CamSensor::Builder& builder,
  const int idx,
  const int64_t ts,
  const cv::Mat& img) const
{
  // Add sensor to outputstate
  builder.setIdx(idx);
  builder.setTimestamp(ts);
  builder.setKey(getName());

  builder.setFocalLengthX(getFocalX());
  builder.setFocalLengthY(getFocalY());
  builder.setPrincipalPointX(getPrincipalPointX());
  builder.setPrincipalPointY(getPrincipalPointY());
  builder.setFovHorizontal(getHorizontalFov());
  builder.setFovVertical(getVerticalFov());

  builder.setX(_tx);
  builder.setY(_ty);
  builder.setZ(_tz);
  builder.setYaw(_yaw);
  builder.setPitch(_pitch);
  builder.setRoll(_roll);

  // Fill img
  builder.getImg().setWidth(img.size().width);
  builder.getImg().setHeight(img.size().height);
  builder.getImg().setChannels(img.channels());
  builder.getImg().setData(
    kj::arrayPtr(img.data, img.size().width * img.size().height * img.channels() * sizeof(uchar))
  );
}
