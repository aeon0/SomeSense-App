#pragma once

#include <tuple>
#include <mutex>
#include <atomic>
#include "icam.h"
#include "../types.h"


namespace data_reader {
  // Base Cam implementation with some implementation needed for all cameras
  // Every new camera should inherit from the BaseCam
  class BaseCam : public ICam {
  public:
    BaseCam(const std::string name, const TS& algoStartTime);

    std::tuple<const bool, const int64_t, cv::Mat> getFrame() override;

    void serialize(
      CapnpOutput::CamSensor::Builder& builder,
      const int idx,
      const int64_t ts,
      const cv::Mat& img
    ) const override;

    cv::Size getFrameSize() const override { return _frameSize; }
    double getFrameRate() const override { return _frameRate; }
    double getHorizontalFov() const override { return _horizontalFov; }
    double getVerticalFov() const override { return _verticalFov; }
    double getFocalX() const override { return _fx; }
    double getFocalY() const override { return _fy; }
    double getPrincipalPointX() const override { return _cx; }
    double getPrincipalPointY() const override { return _cy; }
    float getHorizon() const override { return _horizon; }
    std::tuple<float, float, float> getTranslation() const override { return {_tx, _ty, _tz}; }
    std::tuple<float, float, float> getRotation() const override { return {_pitch, _yaw, _roll}; }
    const cv::Mat& getPoseMat() const override { return _poseMat; }
    const cv::Mat& getCamMat() const override { return _camMat; }
    std::string getName() const override { return _name; }

    // From width, height (in [px]) and horizontalFov (in [rad]), all other intrinsics
    // can be calculated. It assumes the principal point at the center of image (concentric lens)
    void setIntrinsics(const int width, const int height, const double horizontalFov) override;

    // Calcluate pose matrix (rotation, translation, axis flip cam -> autosar) based on extrinsics
    void setExtrinsics(float tx, float ty, float tz, float pitch, float roll, float yaw) override;

    // With known z-coordinate (road height), calculate a 3D point from image coordinate (e.g. for flatworld assumption)
    // Note: imgCoord needs to be relativ to intrinsics of the camera
    cv::Point3f imageToWorldKnownZ(const cv::Point2f& imgCoord, float z = 0) const override;
    cv::Point3f camToWorld(const cv::Point3f& camCoord) const override;
    cv::Point2f worldToImage(const cv::Point3f& worldCoord) const override;
    cv::Point3f imageToCam(const cv::Point2f& imgCoord, float radial_dist) const override;
    float calcLateralAngle(const cv::Point2f& imgCoord) const override;

  protected:
    const std::string _name;
    const TS& _algoStartTime;

    // Intrinsics
    double _horizontalFov; // in [rad]
    double _verticalFov; // in [rad]
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

    // Frame data
    double _frameRate;
    cv::Size _frameSize;
    cv::Mat _bufferFrame;
    cv::Mat _currFrame;
    int64_t _currTs;
    bool _validFrame;

    std::mutex _readMutex;
  };
}
