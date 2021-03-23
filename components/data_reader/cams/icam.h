#pragma once

#include <tuple>
#include <chrono>
#include "opencv2/opencv.hpp"
#include "serialize/frame.capnp.h"


namespace data_reader {
  class ICam {
  public:
    // Get previous frame and returns {success, timestamp in [us], image}
    virtual std::tuple<const bool, const int64_t, cv::Mat> getFrame() = 0;

    // Start reading the camera feed
    virtual void start() = 0;

    // With known z-coordinate (road height), calculate a 3D point from image coordinate (e.g. for flatworld assumption)
    // Note: imgCoord needs to be relativ to intrinsics of the camera
    virtual cv::Point3f imageToWorldKnownZ(const cv::Point2f& imgCoord, float z = 0) const = 0;
    virtual cv::Point3f camToWorld(const cv::Point3f& camCoord) const = 0;
    virtual cv::Point2f worldToImage(const cv::Point3f& worldCoord) const = 0;
    virtual cv::Point3f imageToCam(const cv::Point2f& imgCoord, float radial_dist) const = 0;
    virtual float calcLateralAngle(const cv::Point2f& imgCoord) const = 0;

    // From width, height (in [px]) and horizontalFov (in [rad]), all other intrinsics
    // can be calculated. It assumes the principal point at the center of image (concentric lens)
    virtual void setIntrinsics(const int width, const int height, const double horizontalFov) = 0;

    // Calcluate pose matrix (rotation, translation, axis flip cam -> autosar) based on extrinsics
    virtual void setExtrinsics(float tx, float ty, float tz, float pitch, float roll, float yaw) = 0;

    // Serialize the camera
    virtual void serialize(
      CapnpOutput::CamSensor::Builder& builder,
      const int idx,
      const int64_t ts,
      const cv::Mat& img
    ) const = 0;

    // Return the base (optimal) fps possible for this sensor
    virtual double getFrameRate() const = 0;

    // Return frame size
    virtual cv::Size getFrameSize() const = 0;

    // Return horizontal field of view for camera in [rad]
    virtual double getHorizontalFov() const = 0;

    // Return vertical field of view for camera in [rad]
    virtual double getVerticalFov() const = 0;

    // Return focal length in x direction in [px]
    virtual double getFocalX() const = 0;

    // Return focal length in y direction in [px]
    virtual double getFocalY() const = 0;

    // Return principal point x in [px]
    virtual double getPrincipalPointX() const = 0;

    // Return principal point y in [px]
    virtual double getPrincipalPointY() const = 0;

    // Return horizon in [px] of y-axis in image
    virtual float getHorizon() const = 0;

    // Return [tx, ty, tz] in [px]
    virtual std::tuple<float, float, float> getTranslation() const = 0;

    // Return [pitch, yaw, roll] in [rad]
    virtual std::tuple<float, float, float> getRotation() const = 0;

    // Return pose matrix in [rad] and [m]
    virtual const cv::Mat& getPoseMat() const = 0;
    
    // Return cam matrix in [px]
    virtual const cv::Mat& getCamMat() const = 0;

    // Return a name for the sensor (used for storing)
    virtual std::string getName() const = 0;

    // Return if the camera is based on video data
    virtual bool isRecording() const { return false; }

    // Return rec length in us
    virtual int64_t getRecLength() const { return 0; }

  };
}
