#include "cam_calib.h"
#include "algo/inference/params.h"
#include <cmath>
#include <iostream>


cam_calib::CamCalib::CamCalib(frame::RuntimeMeasService& runtimeMeasService, data_reader::ICam& cam) :
  _runtimeMeasService(runtimeMeasService), _cam(cam)
{
  reset();
}

void cam_calib::CamCalib::reset() {
  auto [tx, ty, tz] = _cam.getTranslation();
  auto [pitch, yaw, roll] = _cam.getRotation();
  _pitch = pitch;
  _tz = tz;
}

void cam_calib::CamCalib::calibrate(const cv::Mat& semseg, const cv::Mat& depth, const util::img::Roi& roi) {
  _runtimeMeasService.startMeas("calib");

  // In a certain grid area, check in the semseg matrix if it corresponds to a the "drivable ground"
  // then check the depth map for the 3D point in sensor coordinates

  const cv::Point gridPointTop(int(depth.size().width / 2), int(depth.size().height * 0.55));
  const cv::Point gridPointBottom(int(depth.size().width / 2), int(depth.size().height * 0.93));

  float depthValTop = 0;
  int count = 0;
  for (int i = -4; i <= 4; ++i) {
    cv::Point gridPoint(gridPointTop.x + (i * 10), gridPointTop.y);
    if (semseg.at<uint8_t>(gridPoint) == inference::ROAD) {
      depthValTop += depth.at<float>(gridPoint);
      count++;
    }
  }
  if (count != 0) depthValTop /= count;

  float depthValBottom = 0;
  count = 0;
  for (int i = -4; i <= 4; ++i) {
    cv::Point gridPoint(gridPointBottom.x + (i * 10), gridPointBottom.y);
    if (semseg.at<uint8_t>(gridPoint) == inference::ROAD) {
      depthValBottom += depth.at<float>(gridPoint);
      count++;
    }
  }
  if (count != 0) depthValBottom /= count;

  if (depthValBottom > 0.5 && depthValTop > 0.5) {
    auto gridPointTopImg = util::img::convertToRoi(roi, gridPointTop);
    auto gridPointBottomImg = util::img::convertToRoi(roi, gridPointBottom);
    cv::Point3f Q = _cam.imageToCam(gridPointTopImg, depthValTop);
    cv::Point3f S = _cam.imageToCam(gridPointBottomImg, depthValBottom);
    cv::Point3f R(S.x - 1.0, S.y, S.z);
    cv::Point3f QR = R - Q;
    cv::Point3f QS = S - Q;
    cv::Point3f N = QR.cross(QS);
    cv::Point3f n = util::img::unit(N);

    // Calc tZ translation - not used as it is currently quite nosiy, TODO: Think about different grid points here
    // https://mathinsight.org/distance_point_plane
    float newTz = n.dot(-S);

    // Calc Pitch
    cv::Point2f pitchVec = util::img::unit(cv::Point2f(n.y, n.z));
    float newPitch = acos(pitchVec.dot(cv::Point2f(0, 1))) - M_PI_2;

    // Calc Roll - not used as it is currently super nosiy, TODO: Take grid points further appart
    // cv::Point2f rollVec = util::img::unit(cv::Point2f(n.x, n.y));
    // float newRoll = acos(rollVec.dot(cv::Point2f(0, 1))) - M_PI;

    // Filter pitch value
    float deltaPitch = _pitch - (0.5 * _pitch + 0.5 * newPitch);
    deltaPitch = std::clamp(deltaPitch, -0.001F, 0.001F);
    _pitch -= deltaPitch;

    // Filter tz value
    float deltaTz = _tz - (0.5 * _tz + 0.5 * newTz);
    deltaTz = std::clamp(deltaTz, -0.01F, 0.01F);
    _tz -= deltaTz;

    auto [tx, ty, tz] = _cam.getTranslation();
    auto [pitch, yaw, roll] = _cam.getRotation();
    // _cam.setExtrinsics(tx, ty, _tz, _pitch, roll, yaw);

    // std::cout << "newtZ: " << newTz << std::endl;
    // std::cout << "tZ: " << _tz << std::endl;
    // std::cout << "newPitch: " << newPitch * (180.0/3.141592653589793238463) << std::endl;
    // std::cout << "pitch: " << _pitch * (180.0/3.141592653589793238463) << std::endl;
    // std::cout << "----" << std::endl;
  }

  _runtimeMeasService.endMeas("calib");
}
