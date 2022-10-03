#include <iostream>
#include <fstream>
#include "util/runtime_meas_service.h"
#include "util/json.hpp"
#include "frame.pb.h"
#include "util/time.h"
#include "util/cam.h"
#include "util/img.h"
#include "util/proto.h"
#include "data/sensor_storage.h"
#include "opencv2/opencv.hpp"
#include <chrono>
#include <thread>
#include <atomic>
using namespace std::chrono_literals;

#ifdef USE_ECAL // Set by CMake
#include "frame/ecal/ecal_nodes.h"
#else
#include "frame/custom/tcp_server.h"
#endif
#include "frame.pb.h"


std::atomic<int> c;
enum {
  CMD_NONE = 0,
  CMD_BREAK,
  CMD_UP,
  CMD_DOWN,
  CMD_SAVE
};

void OnEnter()
{
  while (true)
  {
    auto res = getchar();
    if (res == 27) c = CMD_BREAK;
    if (res == 115) c = CMD_SAVE;
    if (res == 110) c = CMD_DOWN;
    if (res == 109) c = CMD_UP;
    std::cout << res << std::endl;
  }
}


int main(int argc, char** argv) {
  std::cout << "** Start Manual Calibration **" << std::endl;
  std::cout << "Press 'esc' to quit" << std::endl;
  std::cout << "Press 's' to save current calib" << std::endl;
  std::cout << "Press 'n' and 'm' arrow to change horizon" << std::endl;
  std::cout << "Press 'j' to play/pause" << std::endl;
  std::cout << "For every command Enter has to be pressed" << std::endl;
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
  proto::Frame frame;
  util::img::Roi roi{0, 0, 1.0};

  #ifdef USE_ECAL
    frame::initEcal();
    auto com = frame::EcalNodes();
  #else
    auto com = frame::TcpServer();
  #endif

  std::thread kbInput(OnEnter);

  double preDefinedX = -0.5;
  double preDefinedY = 0.0;
  double preDefinedZ = 1.5;

  while (1)
  {
    
    if (play) {
      play = false;
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
    cv::line(drawImg, cv::Point(0, horizon), cv::Point(img.size().width, horizon), cv::Scalar(255, 0, 255), 2);
    
    // This only works with screen attached, on devboard we dont have that
    // cv::imshow("Frame", drawImg);
    auto protoImg = frame.mutable_camsensors(0)->mutable_img();
    auto calib = frame.mutable_camsensors(0)->mutable_calib();
    calib->set_pitch(cam.calcPitchFromHorizon(horizon));
    calib->set_x(preDefinedX);
    calib->set_y(preDefinedY);
    calib->set_z(preDefinedZ);
    util::fillProtoImg<uchar>(protoImg, drawImg, roi);
    com.sendFrame(frame);

    if (c == CMD_BREAK) break; // esc
    else if (c == 32) {
      play = !play; // space
    }
    else if (c == CMD_UP) {
      horizon++; // arrow up
      auto p = cam.calcPitchFromHorizon(horizon) * (180.0/3.141592653589793238463);
      std::cout << p << " deg" << std::endl;
    }
    else if (c == CMD_DOWN) {
      horizon--; // arrow down
      auto p = cam.calcPitchFromHorizon(horizon) * (180.0/3.141592653589793238463);
      std::cout << p << " deg" << std::endl;
    }
    else if (c == CMD_SAVE) {
      nlohmann::json calibJson = {
        {"pitch", cam.calcPitchFromHorizon(horizon)},
        {"yaw", 0.0},
        {"roll", 0.0},
        {"x", preDefinedX},
        {"y", preDefinedY},
        {"z", preDefinedZ},
      };
      std::system("mkdir -p ./tmp");
      std::string fileName = "./tmp/manual_calib.json";
      std::ofstream outputJsonFile(fileName, std::ios::trunc);
      outputJsonFile << calibJson.dump();
      std::cout << "Saved current calibration to: " <<  fileName << std::endl;
    }
    c = CMD_NONE;
    std::this_thread::sleep_for(20ms);
  }

  kbInput.detach();

  // cv::destroyAllWindows();
  return 0;
}
