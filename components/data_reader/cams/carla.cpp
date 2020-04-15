#include "carla.h"
#include <iostream>
#include <thread>
#include <chrono>
#include "../sim/example_scene.hpp"


data_reader::Carla::Carla(const std::string name, const TS& algoStartTime):
    BaseCam(name, algoStartTime) {
  // Setup client
  const std::string host = "localhost";
  const uint16_t port = 2000;
  auto client= carla::client::Client(host, port);
  std::cout << "Connecting to Carla..." << std::endl;
  client.SetTimeout(10s);
  std::cout << "Successfully connected to Carla" << std::endl;

  // Change scene here
  _scene = std::shared_ptr<sim::IScene>(new sim::ExampleScene());
  _scene->setup(client);

  // Listen and read from the camera
  _rgbCam = _scene->getRgbCam();
  int width = 0;
  int height = 0;
  double horizontalFov = 0.0;
  for (auto attrib: _rgbCam->GetAttributes()) {
    if (attrib.GetId() == "image_size_x") {
      int width = std::stoi(attrib.GetValue());
    }
    else if(attrib.GetId() == "image_size_y") {
      int height = std::stoi(attrib.GetValue());
    }
    else if(attrib.GetId() == "sensor_tick") {
      _frameRate = 1.0 / std::stod(attrib.GetValue());
    }
    else if(attrib.GetId() == "fov") {
      horizontalFov = std::stod(attrib.GetValue()) * (M_PI / 180.0);
    }
  }
  setCamIntrinsics(width, height, horizontalFov);
}

void data_reader::Carla::start() {
  _rgbCam->Listen(std::bind(&data_reader::Carla::readRgbCameraData, this, std::placeholders::_1));
}

void data_reader::Carla::readRgbCameraData(carla::SharedPtr<carla::sensor::SensorData> sensorData) {
  const auto captureTime = std::chrono::high_resolution_clock::now();

  auto imagePtr = boost::static_pointer_cast<carla::sensor::data::Image>(sensorData);
  assert(imagePtr != nullptr && "Carla: Error getting image data from rgb sensor");
  auto width = imagePtr->GetWidth();
  auto height = imagePtr->GetHeight();
  cv::Mat bufferFrame(height, width, CV_8UC4, imagePtr->begin());
  cv::cvtColor(bufferFrame, bufferFrame, cv::COLOR_BGRA2BGR);

  std::lock_guard<std::mutex> lockGuard(_readMutex);
  _currTs = static_cast<int64_t>(std::chrono::duration<double, std::micro>(captureTime - _algoStartTime).count());
  _currFrameNr += 1;
  _currFrame = bufferFrame.clone();
  bufferFrame.release();
  _validFrame = true;

  // cv::namedWindow("Debug Carla Cam", cv::WINDOW_AUTOSIZE);
  // cv::imshow("Debug Carla Cam Img", _currFrame);
  // cv::waitKey(1);
}
