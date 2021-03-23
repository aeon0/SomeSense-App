#include "base_cam.h"
#include <math.h>


data_reader::BaseCam::BaseCam(const std::string name, const TS& algoStartTime) : 
  _name(name), _algoStartTime(algoStartTime), _currTs(-1), _validFrame(false) 
{
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
    1, 0, 0, _tx,
    0, 1, 0, _ty,
    0, 0, 1, _tz,
    0, 0, 0,  1);
  // Flip axis (camera -> autosar)
  cv::Mat flipAxis = (cv::Mat_<float>(4, 4) <<
    0, -1,  0, 0,
    0,  0, -1, 0,
    1,  0,  0, 0,
    0,  0,  0, 1);

  _poseMat = flipAxis * rotX * rotY * rotZ * trans;
  _poseMatTrans = _poseMat.t();

  // Calc Horizon from pitch value by rotating the optical axis by -pitch to have a vector parallel to the world plane
  cv::Mat opticalAxis = (cv::Mat_<float>(4, 1) << 1, 0, 0, 0);
  cv::Mat p = _camMat * flipAxis * rotY * opticalAxis;
  _horizon = p.at<float>(1) / p.at<float>(2);
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
  _camMatTrans = _camMat.t();
}

cv::Point3f data_reader::BaseCam::imageToWorldKnownZ(const cv::Point2f& imgCoord, float z) const {
  cv::Point2f normImgCoord((imgCoord.x - _cx) * (1 / _fx), (imgCoord.y - _cy) * (1 / _fy));

  // Rotate the norm cam coordinates (x, y, 1) to world coordinate system
  cv::Mat normImgMat = (cv::Mat_<float>(4, 1) << normImgCoord.x, normImgCoord.y, 1, 0);
  cv::Mat rotatedCamMat = _poseMatTrans * normImgMat;

  const float zCam = (z - _tz) * (1 / rotatedCamMat.at<float>(2));
  const float x = rotatedCamMat.at<float>(0) * zCam + _tx;
  const float y = rotatedCamMat.at<float>(1) * zCam + _ty;

  cv::Point3f result(x, y, z);
  return result;
}

cv::Point3f data_reader::BaseCam::camToWorld(const cv::Point3f& camCoord) const {
  cv::Mat camCoordMat = (cv::Mat_<float>(4, 1) << camCoord.x, camCoord.y, camCoord.z, 0);
  cv::Mat worldCoordMat = _poseMatTrans * camCoordMat;
  cv::Point3f worldCoord(worldCoordMat.at<float>(0), worldCoordMat.at<float>(1), worldCoordMat.at<float>(2));
  return worldCoord;
}

cv::Point3f data_reader::BaseCam::imageToCam(const cv::Point2f& imgCoord, float radial_dist) const {
  // 3D Cam vector that goes through z=1
  cv::Point2f normImgCoord((imgCoord.x - _cx) * (1 / _fx), (imgCoord.y - _cy) * (1 / _fy));
  // Normalize length of 3D vector to 1 to get unit vector
  const float length = sqrt(pow(normImgCoord.x, 2) + pow(normImgCoord.y, 2) + 1.0);
  cv::Point3f camCoord(normImgCoord.x / length, normImgCoord.y / length, 1.0 / length);
  camCoord *= radial_dist;
  return camCoord;
}

float data_reader::BaseCam::calcLateralAngle(const cv::Point2f& imgCoord) const
{
  const float deltaX = (_cx - imgCoord.x);
  const float latAngel = atan(deltaX / _fy);
  return latAngel;
}

cv::Point2f data_reader::BaseCam::worldToImage(const cv::Point3f& worldCoord) const {
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
  builder.setHorizon(getHorizon());

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
