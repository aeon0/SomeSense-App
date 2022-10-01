#pragma once
#include "camera.pb.h"
#include "opencv2/opencv.hpp"


namespace util {
  class Cam {
  public:
    Cam();

    void initFromProto(const proto::CamCalibration& calib);
    // From width, height (in [px]) and horizontalFov (in [rad]), all other intrinsics
    // can be calculated. It assumes the principal point at the center of image (concentric lens)
    void setIntrinsics(const int width, const int height, const double horizontalFov);
    void setIntrinsics(const proto::CamCalibration& calib);
    // Calcluate pose matrix (rotation, translation, axis flip cam -> autosar) based on extrinsics
    void setExtrinsics(float tx, float ty, float tz, float pitch, float roll, float yaw);
    void setExtrinsics(const proto::CamCalibration& calib);

    void fillProtoCalib(proto::CamCalibration* calib);
    const cv::Mat& getPoseMat() const { return _poseMat; }
    const cv::Mat& getCamMat() const { return _camMat; }

    cv::Point3f imageToWorldKnownZ(const cv::Point2f& imgCoord, float z = 0) const;
    cv::Point3f camToWorld(const cv::Point3f& camCoord) const;
    cv::Point2f worldToImage(const cv::Point3f& worldCoord) const;
    cv::Point3f worldToCam(const cv::Point3f& worldCoord) const;
    cv::Point3f imageToCam(const cv::Point2f& imgCoord, float radial_dist) const;
    float calcLateralAngle(const cv::Point2f& imgCoord) const;

    double calcPitchFromHorizon(int horizon) const;
    double calcHorizonFromPitch(double pitch) const;

    double getHorizon() const { return _horizon; }
  private:
    void calcCamMat();

    // Intrinsics
    double _fx; // focal length x in [px]
    double _fy; // focal length y in [px]
    double _cx; // principal point x in [px]
    double _cy; // principal point y in [px]
    cv::Mat _camMat;
    cv::Mat _camMatTrans;

    // Extrinsics
    float _tx; // translation autosar-x in [m] with (0, 0) at ego vehicle bumper
    float _ty; // translation autosar-y in [m] with (0, 0) at ego vehicle bumper
    float _tz; // translation autosar-z in [m] with (0, 0) at ego vehicle bumper
    float _pitch; // in [rad]
    float _roll; // in [rad]
    float _yaw; // in [rad]
    float _horizon; // in [px] y-value
    cv::Mat _poseMat;
    cv::Mat _poseMatTrans;
    cv::Mat _poseMatInv;
    cv::Mat _rotX;
    cv::Mat _rotY;
    cv::Mat _rotZ;
    cv::Mat _flipAxis;
    cv::Mat _trans;
  };
}
