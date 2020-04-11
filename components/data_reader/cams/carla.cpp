#include "carla.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <carla/client/Client.h>
#include <carla/client/Map.h>
#include <carla/client/Sensor.h>
#include <carla/client/World.h>
#include <carla/geom/Transform.h>
#include <carla/image/ImageIO.h>
#include <carla/image/ImageView.h>
#include <carla/sensor/data/Image.h>


data_reader::Carla::Carla(const std::string name, const TS& algoStartTime):
    BaseCam(name, algoStartTime) {
  // Setup client
  const std::string host = "localhost";
  const uint16_t port = 2000;
  auto client= carla::client::Client(host, port);
  client.SetTimeout(10s);

  auto availableMaps = client.GetAvailableMaps();
  // std::cout << "Loading world: " << town_name << std::endl;

  // auto world = client.LoadWorld(town_name);
  // auto transform = RandomChoice(map->GetRecommendedSpawnPoints(), rng);

  // _frameRate = _cam.get(cv::CAP_PROP_FPS);
  // _frameSize = cv::Size(_cam.get(cv::CAP_PROP_FRAME_WIDTH), _cam.get(cv::CAP_PROP_FRAME_HEIGHT));

  // Start thread to read image and store it into _currFrame
  std::thread dataReaderThread(&data_reader::Carla::readData, this);
  dataReaderThread.detach();
}

void data_reader::Carla::readData() {
  for (;;) {
    const auto captureTime = std::chrono::high_resolution_clock::now();

    // bool success = _cam.read(_bufferFrame);

    std::lock_guard<std::mutex> lockGuard(_readMutex);
    _currTs = static_cast<int64_t>(std::chrono::duration<double, std::micro>(captureTime - _algoStartTime).count());
    _currFrameNr += 1;
    _currFrame = _bufferFrame.clone();
    _bufferFrame.release();
    _validFrame = true;
  }
}
