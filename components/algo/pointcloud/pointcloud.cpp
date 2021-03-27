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
  // cv::Mat semsegImg = cv::Mat(semseg.size().height, semseg.size().width, CV_8UC3, cv::Scalar(2));
  // cv::Mat depthImg = cv::Mat(depth.size().height, depth.size().width, CV_8UC1, cv::Scalar(0));

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
        if (!foundMovable && (y > 10)) {
          // Get avg world z-coordinate from the previous 5 pixels
          // Once the "plane" afterwards has reached a certain height, we stop looking further up
          const int start = std::max(0, y + 3);
          const int end = std::max(1, y + 8);
          for (int i = start; i < end; ++i) {
            cv::Point3f worldCoord = cam.camToWorld(cam.imageToCam(util::img::convertToRoi(roi, cv::Point(x, i)), radialDistCam));
            previousZ += worldCoord.z;
          }
          previousZ /= (end - start);
        }
        if (!foundMovable) {
          // Flat world point
          const cv::Point3f worldCoordFW = cam.imageToWorldKnownZ(util::img::convertToRoi(roi, cv::Point2f(x, y)), 0);
          _obstacles.push_back(worldCoordFW);
          cv::Point3f camCoordFw = cam.worldToCam(worldCoordFW);
          radialDistFW = sqrt(pow(camCoordFw.x, 2.0) + pow(camCoordFw.y, 2.0) + pow(camCoordFw.z, 2.0));
        }
        cv::Point3f worldCoord = cam.camToWorld(cam.imageToCam(util::img::convertToRoi(roi, cv::Point(x, y)), radialDistCam * 0.35F + radialDistFW * 0.65F));
        // Check X-Gradient, there shouldnt be any hard gradients to the left or right
        auto distLeft = depth.at<float>(y, x-1);
        auto distRight = depth.at<float>(y, x+1);
        float maxGrad = std::max(std::abs(radialDistCam - distLeft), std::abs(radialDistCam - distRight)) / radialDistCam;
        if (worldCoord.z < (previousZ + 1.6F) && worldCoord.z < 4.0F && maxGrad < 0.1F) {
          _obstacles.push_back(worldCoord);
        }
        else {
          sampleY = false;
        }
        foundMovable = true;
      }
      else if (foundMovable) {
        sampleY = false;
      }

      if (y < int(minHorizon.y) || semsegClass == inference::UNDRIVEABLE) {
        sampleY = false;
      }
      else {
        y--;
      }

      // semsegImg.at<cv::Vec3b>(y, x) = inference::COLORS_SEMSEG[semsegClass];
      // depthImg.at<uint8_t>(y, x) = (uint8_t)(radialDistCam);
    }
  }
  _runtimeMeasService.endMeas("pointcloud");
  _runtimeMeasService.printToConsole();

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
