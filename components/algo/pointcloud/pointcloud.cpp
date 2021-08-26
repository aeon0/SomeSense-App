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
  cv::Mat semsegImg = cv::Mat(semseg.size().height, semseg.size().width, CV_8UC3, cv::Scalar(2));
  cv::Mat depthImg = cv::Mat(depth.size().height, depth.size().width, CV_8UC1, cv::Scalar(0));

  _runtimeMeasService.startMeas("pointcloud");
  const cv::Point2f minHorizon = util::img::convertToRoiInv(roi, cv::Point2f(0.0, cam.getHorizon()));
  for (int x = 1; x < semseg.size().width - 1; ++x) {
    bool foundMovable = false;
    float previousZ = 0.0F;
    float radialDistFW = 0.0F;
    bool sampleY = true;
    int y = (semseg.size().height - 1);
    while (sampleY) {
      auto semsegClass = semseg.at<uint8_t>(y, x);
      auto radialDistCam = depth.at<float>(y, x);

      if (semsegClass == inference::LANE_MARKINGS) {
        const cv::Point3f worldCoord = cam.imageToWorldKnownZ(util::img::convertToRoi(roi, cv::Point2f(x, y)), 0);
        // const cv::Point3f worldCoord = cam.camToWorld(cam.imageToCam(util::img::convertToRoi(roi, cv::Point(x, y)), radialDistCam));
        _laneMarkings.push_back(worldCoord);
      }
      else if (semsegClass == inference::MOVABLE) {
        if (!foundMovable)
        {
          // First movable, thus must be bottom edge (assuming no occlusion)
          const cv::Point3f worldCoordFW = cam.imageToWorldKnownZ(util::img::convertToRoi(roi, cv::Point2f(x, y)), 0);
          _obstacles.push_back(worldCoordFW);
          cv::Point3f camCoordFw = cam.worldToCam(worldCoordFW);
          radialDistFW = sqrt(pow(camCoordFw.x, 2.0) + pow(camCoordFw.y, 2.0) + pow(camCoordFw.z, 2.0));
        }

        // Dont do depth at the image edges as there is somewhat of a "bleeding"
        if (x > (depth.size().width * 0.1) && x < (depth.size().width * 0.9) && y > (depth.size().height * 0.1) && y < (depth.size().height * 0.9))
        {
          float weightedDist = radialDistCam * 0.4 + radialDistFW * 0.6;
          cv::Point3f worldCoordDepth = cam.camToWorld(cam.imageToCam(util::img::convertToRoi(roi, cv::Point(x, y)), weightedDist));
          auto distLeft = depth.at<float>(y, x-1);
          auto distRight = depth.at<float>(y, x+1);
          auto distTop = depth.at<float>(y-1, x);
          auto distBottom = depth.at<float>(y+1, x);
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

      semsegImg.at<cv::Vec3b>(y, x) = inference::COLORS_SEMSEG[semsegClass];
      depthImg.at<uint8_t>(y, x) = (uint8_t)(radialDistCam);

      if (y == 1 || y < int(minHorizon.y) || semsegClass == inference::UNDRIVEABLE) {
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
