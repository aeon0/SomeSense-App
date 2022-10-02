#include "pointcloud.h"
#include "algo/inference/params.h"
#include <iostream>
#include "util/proto.h"


algo::Pointcloud::Pointcloud(util::RuntimeMeasService& runtimeMeasService) :
  _runtimeMeasService(runtimeMeasService)
{
  reset();
}

void algo::Pointcloud::reset() {
  _obstacles.clear();
  _laneMarkings.clear();
}

void algo::Pointcloud::run(const proto::Frame& frame) {
  // Note: We currently just assume that we have only one front facing camera
  //       Does not support multi camera setup right now!

  auto camSensor = frame.camsensors(0);
  auto t1 = camSensor.semsegraw();
  auto t2 = camSensor.semsegimg();
  util::fillCvImg(_semseg, camSensor.semsegraw());
  util::fillCvImg(_depth, camSensor.depthraw());
  _cam.initFromProto(camSensor.calib());
  // depth and semseg have the same roi
  _roi.offsetLeft = camSensor.semsegraw().offsetleft();
  _roi.offsetTop = camSensor.semsegraw().offsettop();
  _roi.scale = camSensor.semsegraw().scale();

  reset();
  cv::Mat semsegImg = cv::Mat(_semseg.size().height, _semseg.size().width, CV_8UC3, cv::Scalar(2));
  cv::Mat depthImg = cv::Mat(_depth.size().height, _depth.size().width, CV_8UC1, cv::Scalar(0));

  _runtimeMeasService.startMeas("pointcloud");
  const cv::Point2f minHorizon = util::img::convertToRoiInv(_roi, cv::Point2f(0.0, _cam.getHorizon()));
  for (int x = 1; x < _semseg.size().width - 1; ++x) {
    bool foundMovable = false;
    float previousZ = 0.0F;
    float radialDistFW = 0.0F;
    bool sampleY = true;
    int y = (_semseg.size().height - 1);
    while (sampleY) {
      auto semsegClass = _semseg.at<uint8_t>(y, x);
      auto radialDistCam = _depth.at<float>(y, x);

      if (semsegClass == LANE_MARKINGS) {
        const cv::Point3f worldCoord = _cam.imageToWorldKnownZ(util::img::convertToRoi(_roi, cv::Point2f(x, y)), 0);
        // const cv::Point3f worldCoord = cam.camToWorld(cam.imageToCam(util::img::convertToRoi(roi, cv::Point(x, y)), radialDistCam));
        _laneMarkings.push_back(worldCoord);
      }
      else if (semsegClass == MOVABLE) {
        if (!foundMovable)
        {
          // First movable, thus must be bottom edge (assuming no occlusion)
          const cv::Point3f worldCoordFW = _cam.imageToWorldKnownZ(util::img::convertToRoi(_roi, cv::Point2f(x, y)), 0);
          _obstacles.push_back(worldCoordFW);
          cv::Point3f camCoordFw = _cam.worldToCam(worldCoordFW);
          radialDistFW = sqrt(pow(camCoordFw.x, 2.0) + pow(camCoordFw.y, 2.0) + pow(camCoordFw.z, 2.0));
        }

        // Dont do depth at the image edges as there is somewhat of a "bleeding"
        if (x > (_depth.size().width * 0.1) && x < (_depth.size().width * 0.9) && y > (_depth.size().height * 0.1) && y < (_depth.size().height * 0.9))
        {
          float weightedDist = radialDistCam * 0.4 + radialDistFW * 0.6;
          cv::Point3f worldCoordDepth = _cam.camToWorld(_cam.imageToCam(util::img::convertToRoi(_roi, cv::Point(x, y)), weightedDist));
          auto distLeft = _depth.at<float>(y, x-1);
          auto distRight = _depth.at<float>(y, x+1);
          auto distTop = _depth.at<float>(y-1, x);
          auto distBottom = _depth.at<float>(y+1, x);
          float maxGradX = std::max(std::abs(radialDistCam - distLeft), std::abs(radialDistCam - distRight)) / radialDistCam;
          float maxGradY = std::max(std::abs(radialDistCam - distTop), std::abs(radialDistCam - distBottom)) / radialDistCam;
          // check x and y gradient and do not use if too large
          if (worldCoordDepth.z < 1.5 && worldCoordDepth.z > -0.05 && maxGradX < 0.1F && maxGradY < 0.1F) {
            _obstacles.push_back(worldCoordDepth);
          }
        }

        foundMovable = true;
      }
      else if (foundMovable) {
        sampleY = false;
      }

      semsegImg.at<cv::Vec3b>(y, x) = COLORS_SEMSEG[semsegClass];
      depthImg.at<uint8_t>(y, x) = (uint8_t)(radialDistCam);

      if (y == 1 || y < int(minHorizon.y) || semsegClass == UNDRIVEABLE) {
        sampleY = false;
      }
      else {
        y--;
      }
    }
  }
  _runtimeMeasService.endMeas("pointcloud");

  // cv::imshow("semseg", semsegImg);
  // cv::imshow("depth", depthImg);
  // cv::waitKey(1);
}

void algo::Pointcloud::serialize(proto::Frame& frame) {
  // Fill obstacles
  for (int i = 0; i < _obstacles.size(); ++i) {
    auto o = frame.mutable_obstacles()->Add();
    o->set_x(_obstacles[i].x);
    o->set_y(_obstacles[i].y);
    o->set_z(_obstacles[i].z);
  }
  // Fill lane markings
  for (int i = 0; i < _laneMarkings.size(); ++i) {
    auto l = frame.mutable_lanemarkings()->Add();
    l->set_x(_laneMarkings[i].x);
    l->set_y(_laneMarkings[i].y);
    l->set_z(_laneMarkings[i].z);
  }
}
