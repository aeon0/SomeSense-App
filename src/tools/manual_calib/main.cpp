#include <iostream>
#include <fstream>
#include "util/runtime_meas_service.h"
#include "util/json.hpp"
#include "frame.pb.h"
#include "util/time.h"
#include "util/cam.h"
#include "util/proto.h"
#include "data/sensor_storage.h"
#include "opencv2/opencv.hpp"


int main(int argc, char** argv) {
  std::cout << "** Start Manual Calibration **" << std::endl;
  std::cout << "Press 'esc' to quit" << std::endl;
  std::cout << "Press 's' to save current calib" << std::endl;
  std::cout << "Press 'up' and 'down' arrow to change horizon" << std::endl;
  std::cout << "Press 'space' to play/pause" << std::endl;
  std::cout << std::endl;

  // Creating Runtime Meas Service
  auto runtimeMeasService = util::RuntimeMeasService();

  // Create Sensor Storage
  assert(argc == 2 && "Missing argument for config path");
  auto sensorStorage = data::SensorStorage(runtimeMeasService);
  const std::string sensorConfigPath = argv[1];
  sensorStorage.createFromConfig(sensorConfigPath);
  auto appStartTime = std::chrono::high_resolution_clock::now();

  // actions
  cv::Mat img;
  bool play = true;
  int horizon = -1;

  auto cam = util::Cam();

  while (1)
  {
    if (play) {
      play = false;
      proto::Frame frame;
      sensorStorage.fillFrame(frame, appStartTime);
      // Just taking first index of camsensor
      cv::Mat tmpImg;
      util::fillCvImg(tmpImg, frame.camsensors(0).img());
      img = tmpImg.clone();
      cam.initFromProto(frame.camsensors(0).calib());
      if (horizon == -1) {
        horizon = int(img.size().height / 2);
      }
    }

    cv::Mat drawImg = img.clone();
    cv::line(drawImg, cv::Point(0, horizon), cv::Point(img.size().width, horizon), cv::Scalar(255, 0, 255), 1);
    cv::imshow("Frame", drawImg);

    auto c = cv::waitKey(25);
    // std::cout << c << std::endl;
    if (c == 27) break; // esc
    else if (c == 32) play = !play; // space
    else if (c == 84) {
      horizon++; // arrow up
      auto p = cam.calcPitchFromHorizon(horizon) * (180.0/3.141592653589793238463);
      std::cout << p << " deg" << std::endl;
    }
    else if (c == 82) {
      horizon--; // arrow down
      auto p = cam.calcPitchFromHorizon(horizon) * (180.0/3.141592653589793238463);
      std::cout << p << " deg" << std::endl;
    }
    else if (c == 115) {
      nlohmann::json calibJson = {
        {"pitch", cam.calcPitchFromHorizon(horizon)},
        {"yaw", 0.0},
        {"roll", 0.0},
        {"x", -0.5},
        {"y", 0.0},
        {"z", 1.5},
      };
      std::system("mkdir -p ./tmp");
      std::string fileName = "./tmp/manual_calib.json";
      std::ofstream outputJsonFile(fileName, std::ios::trunc);
      outputJsonFile << calibJson.dump();
      std::cout << "Saved current calibration to: " <<  fileName << std::endl;
    }
  }

  cv::destroyAllWindows();
  return 0;
}
