#include "pointcloud.h"
#include "algo/inference/params.h"
#include <iostream>


pointcloud::Pointcloud::Pointcloud(frame::RuntimeMeasService& runtimeMeasService) :
  _runtimeMeasService(runtimeMeasService)
{
  reset();
}

void pointcloud::Pointcloud::reset() {
  _obstacles.clear();
  _laneMarkings.clear();
}

void pointcloud::Pointcloud::processData(const cv::Mat& semseg, const cv::Mat& depth, const util::img::Roi& roi, const data_reader::ICam& cam) {
  reset();
  _runtimeMeasService.startMeas("pointcloud");
  cv::Mat test(semseg.size(), CV_32FC3);
  for (int y = 0; y < semseg.size().height; ++y) {
    for (int x = 0; x < semseg.size().width; ++x) {
      auto radialDistCam = depth.at<float>(y, x);
      auto semsegClass = semseg.at<inference::SemsegCls>(y, x);
      // Convert to world coordinates
      cv::Point3f camCoord = cam.imageToCam(util::img::convertToRoi(roi, cv::Point(x, y)), radialDistCam);
      test.at<cv::Vec3f>(y, x) = cv::Vec3b(camCoord.x, camCoord.y, camCoord.z);
      cv::Point3f worldCoord = cam.camToWorld(camCoord);
      // if (semsegClass == inference::LANE_MARKINGS) {
      //   _laneMarkings.push_back(worldCoord);
      // }
      // else {
      //   _obstacles.push_back(worldCoord);
      // }
    }
  }
  _runtimeMeasService.endMeas("pointcloud");
  _runtimeMeasService.printToConsole();
}

void pointcloud::Pointcloud::serialize(CapnpOutput::Frame::Builder& builder) {
  // Fill obstacles
  auto obstacles = builder.initObstacles(_obstacles.size());
  for (int i = 0; i < _obstacles.size(); ++i) {
    obstacles[i].setX(_obstacles[i].x);
    obstacles[i].setY(_obstacles[i].y);
    obstacles[i].setZ(_obstacles[i].z);
  }
  // Fill lane markings
  auto laneMarkings = builder.initLaneMarkings(_laneMarkings.size());
  for (int i = 0; i < _laneMarkings.size(); ++i) {
    laneMarkings[i].setX(_laneMarkings[i].x);
    laneMarkings[i].setY(_laneMarkings[i].y);
    laneMarkings[i].setZ(_laneMarkings[i].z);
  }
}
