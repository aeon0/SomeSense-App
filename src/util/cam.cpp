#include "cam.h"
#include <math.h>
#include <iostream>


util::Cam::Cam() {
}

void util::Cam::fillProtoCalib(proto::CamCalibration* calib) {
  calib->set_horizontalfov(_horizontalFov);
  calib->set_verticalfov(_verticalFov);
  calib->set_focallengthx(_fx);
  calib->set_focallengthy(_fy);
  calib->set_principalpointx(_cx);
  calib->set_principalpointy(_cy);
  calib->set_x(_tx);
  calib->set_y(_ty);
  calib->set_z(_tz);
  calib->set_pitch(_pitch);
  calib->set_yaw(_yaw);
  calib->set_roll(_roll);
  calib->set_horizon(_horizon);
}

void util::Cam::initFromProto(const proto::CamCalibration& calib) {
  setIntrinsics(calib);
  setExtrinsics(calib);
}

void util::Cam::setIntrinsics(const proto::CamCalibration& calib) {
  _fx = calib.focallengthx();
  _fy = calib.focallengthy();
  _cx = calib.principalpointx();
  _cy = calib.principalpointy();
  _verticalFov = calib.verticalfov();
  _horizontalFov = calib.horizontalfov();
  calcCamMat();
}

void util::Cam::setExtrinsics(const proto::CamCalibration& calib) {
  setExtrinsics(calib.x(), calib.y(), calib.z(), calib.pitch(), calib.roll(), calib.yaw());
}

void util::Cam::setIntrinsics(const int width, const int height, const double horizontalFov) {
  assert(horizontalFov > 0.0);
  assert(width > 0);
  assert(height > 0);

  // Set principal point
  // TODO: This might not be always true
  _cx = static_cast<double>(width) / 2.0;
  _cy = static_cast<double>(height) / 2.0;
  // Set focal length
  _fx = (static_cast<double>(width) * 0.5) / tan(horizontalFov * 0.5);
  _fy = _fx;
  // Fov
  _horizontalFov = horizontalFov;
  _verticalFov = atan(height / (2*_fy));

  calcCamMat();
}

void util::Cam::calcCamMat() {
  _camMat = (cv::Mat_<float>(4, 4) <<
  _fx,   0, _cx, 0,
    0, _fy, _cy, 0,
    0,   0,   1, 0,
    0,   0,   0, 1);
_camMatTrans = _camMat.t();
}

void util::Cam::setExtrinsics(float tx, float ty, float tz, float pitch, float roll, float yaw)
{
  _tx = tx;
  _ty = ty;
  _tz = tz;
  _pitch = pitch;
  _roll = roll;
  _yaw = yaw;

  // roll
  _rotX = (cv::Mat_<float>(4, 4) << 
    1,           0,          0, 0,
    0,  cos(_roll), sin(_roll), 0,
    0, -sin(_roll), cos(_roll), 0,
    0,           0,          0, 1);
  // pitch
  _rotY = (cv::Mat_<float>(4, 4) <<
    cos(_pitch), 0, -sin(_pitch), 0,
              0, 1,            0, 0,
    sin(_pitch), 0,  cos(_pitch), 0,
              0, 0,            0, 1);
  // yaw
  _rotZ = (cv::Mat_<float>(4, 4) <<
     cos(_yaw), sin(_yaw), 0, 0,
    -sin(_yaw), cos(_yaw), 0, 0,
             0,         0, 1, 0,
             0,         0, 0, 1);
  // translation
  _trans = (cv::Mat_<float>(4, 4) <<
    1, 0, 0, -_tx,
    0, 1, 0, -_ty,
    0, 0, 1, -_tz,
    0, 0, 0,  1);
  // Flip axis (camera -> autosar)
  _flipAxis = (cv::Mat_<float>(4, 4) <<
    0, -1,  0, 0,
    0,  0, -1, 0,
    1,  0,  0, 0,
    0,  0,  0, 1);

  _poseMat = _flipAxis * _rotX * _rotY * _rotZ * _trans;
  _poseMatTrans = _poseMat.t();
  _poseMatInv = _poseMat.inv();

  _horizon = calcHorizonFromPitch(_pitch);
}

cv::Point3f util::Cam::imageToWorldKnownZ(const cv::Point2f& imgCoord, float z) const {
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

cv::Point3f util::Cam::camToWorld(const cv::Point3f& camCoord) const {
  cv::Mat camCoordMat = (cv::Mat_<float>(4, 1) << camCoord.x, camCoord.y, camCoord.z, 1);
  cv::Mat worldCoordMat = _poseMatInv * camCoordMat;
  cv::Point3f worldCoord(worldCoordMat.at<float>(0), worldCoordMat.at<float>(1), worldCoordMat.at<float>(2));
  return worldCoord;
}

cv::Point3f util::Cam::imageToCam(const cv::Point2f& imgCoord, float radial_dist) const {
  // 3D Cam vector that goes through z=1
  cv::Point2f normImgCoord((imgCoord.x - _cx) * (1 / _fx), (imgCoord.y - _cy) * (1 / _fy));
  // Normalize length of 3D vector to 1 to get unit vector
  const float length = sqrt(pow(normImgCoord.x, 2) + pow(normImgCoord.y, 2) + 1.0);
  cv::Point3f camCoord(normImgCoord.x / length, normImgCoord.y / length, 1.0 / length);
  camCoord *= radial_dist;
  return camCoord;
}

float util::Cam::calcLateralAngle(const cv::Point2f& imgCoord) const
{
  const float deltaX = (_cx - imgCoord.x);
  const float latAngel = atan(deltaX / _fy);
  return latAngel;
}

cv::Point2f util::Cam::worldToImage(const cv::Point3f& worldCoord) const {
  cv::Mat worldCoordMat = (cv::Mat_<float>(4, 1) << worldCoord.x, worldCoord.y, worldCoord.z, 1);
  cv::Mat imgCoordMat = _camMat * _poseMat * worldCoordMat;
  cv::Point2f imgCoord(imgCoordMat.at<float>(0) / imgCoordMat.at<float>(2), imgCoordMat.at<float>(1) / imgCoordMat.at<float>(2));
  return imgCoord;
}

cv::Point3f util::Cam::worldToCam(const cv::Point3f& worldCoord) const {
  cv::Mat worldCoordMat = (cv::Mat_<float>(4, 1) << worldCoord.x, worldCoord.y, worldCoord.z, 1);
  cv::Mat imgCoordMat = _poseMat * worldCoordMat;
  cv::Point3f camCoord(imgCoordMat.at<float>(0), imgCoordMat.at<float>(1), imgCoordMat.at<float>(2));
  return camCoord;
}

double util::Cam::calcPitchFromHorizon(int horizon) const {
  const double p = atan((_cy - horizon) / _fx);
  return p;
}

double util::Cam::calcHorizonFromPitch(double pitch) const {
  const double h = _cy - tan(pitch) * _fx;
  return h;
}
