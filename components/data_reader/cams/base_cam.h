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

    const cv::Size getFrameSize() const override { return _frameSize; }
    const double getFrameRate() const override { return _frameRate; }
    const double getHorizontalFov() const override { return _horizontalFov; }
    const double getVerticalFov() const override { return _verticalFov; }
    const double getFocalX() const override { return _fx; }
    const double getFocalY() const override { return _fy; }
    const double getPrincipalPointX() const override { return _cx; }
    const double getPrincipalPointY() const override { return _cy; }
    const cv::Mat& getPoseMat() const { return _poseMat; }
    const cv::Mat& getCamMat() const { return _camMat; }
    const std::string getName() const override { return _name; }

    // With known z-coordinate (road height), calculate a 3D point from image coordinate (e.g. for flatworld assumption)
    // Note: imgCoord needs to be relativ to intrinsics of the camera
    cv::Point3f imageToWorldKnownZ(cv::Point2f imgCoord, float z = 0) const override;
    cv::Point2f worldToImage(cv::Point3f worldCoord) const override;

  protected:
    // From width, height (in [px]) and horizontalFov (in [rad]), all other intrinsics
    // can be calculated. It assumes the principal point at the center of image (concentric lens)
    void setIntrinsics(const int width, const int height, const double horizontalFov);

    // Calcluate pose matrix (rotation, translation, axis flip cam -> autosar) based on extrinsics
    void setExtrinsics(float tx, float ty, float tz, float pitch, float roll, float yaw);

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

    // Extrinsics
    float _tx; // translation autosar-x in [m] with (0, 0) at ego vehicle bumper
    float _ty; // translation autosar-y in [m] with (0, 0) at ego vehicle bumper
    float _tz; // translation autosar-z in [m] with (0, 0) at ego vehicle bumper
    float _pitch; // in [rad]
    float _roll; // in [rad]
    float _yaw; // in [rad]
    cv::Mat _poseMat;

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
