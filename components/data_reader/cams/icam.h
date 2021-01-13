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
    virtual cv::Point3f imageToWorldKnownZ(cv::Point2f imgCoord, float z = 0) const = 0;
    virtual cv::Point2f worldToImage(cv::Point3f worldCoord) const = 0;

    // Serialize the camera
    virtual void serialize(
      CapnpOutput::CamSensor::Builder& builder,
      const int idx,
      const int64_t ts,
      const cv::Mat& img
    ) const = 0;

    // Return the base (optimal) fps possible for this sensor
    virtual const double getFrameRate() const = 0;

    // Return frame size
    virtual const cv::Size getFrameSize() const = 0;

    // Return horizontal field of view for camera in [rad]
    virtual const double getHorizontalFov() const = 0;

    // Return vertical field of view for camera in [rad]
    virtual const double getVerticalFov() const = 0;

    // Return focal length in x direction in [px]
    virtual const double getFocalX() const = 0;

    // Return focal length in y direction in [px]
    virtual const double getFocalY() const = 0;

    // Return principal point x in [px]
    virtual const double getPrincipalPointX() const = 0;

    // Return principal point y in [px]
    virtual const double getPrincipalPointY() const = 0;

    // Return a name for the sensor (used for storing)
    virtual const std::string getName() const = 0;

    // Return if the camera is based on video data
    virtual const bool isRecording() const { return false; }

    // Return rec length in us
    virtual const int64_t getRecLength() const { return 0; }

  };
}
