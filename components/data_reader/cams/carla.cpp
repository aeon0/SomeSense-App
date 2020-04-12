#include "carla.h"
#include <iostream>
#include <thread>
#include <chrono>


data_reader::Carla::Carla(const std::string name, const TS& algoStartTime):
    BaseCam(name, algoStartTime) {
  // Setup client
  const std::string host = "localhost";
  const uint16_t port = 2000;
  auto client= carla::client::Client(host, port);
  std::cout << "Connecting to Carla..." << std::endl;
  client.SetTimeout(10s);
  std::cout << "Success: Carla Connection" << std::endl;

  auto availableMaps = client.GetAvailableMaps();
  auto townName = availableMaps[0];
  std::cout << "Loading world: " << townName << std::endl;
  auto world = client.LoadWorld(townName);
  auto map = world.GetMap();
  auto transform = map->GetRecommendedSpawnPoints()[0];
  auto blueprintLib = world.GetBlueprintLibrary();
  auto vehicles = blueprintLib->Filter("vehicle");
  auto blueprint = vehicles->at(0);
  auto actor = world.SpawnActor(blueprint, transform);
  _egoVehicle = boost::static_pointer_cast<carla::client::Vehicle>(actor);

  // Apply control to vehicle
  carla::client::Vehicle::Control control;
  control.throttle = 1.0f;
  _egoVehicle->ApplyControl(control);

  // Move spectator so we can see the vehicle from the simulator window.
  auto spectator = world.GetSpectator();
  transform.location += 32.0f * transform.GetForwardVector();
  transform.location.z += 2.0f;
  transform.rotation.yaw += 180.0f;
  transform.rotation.pitch = -15.0f;
  spectator->SetTransform(transform);

  // Find a rgb camera, set params
  auto cameraBpPtr = blueprintLib->Find("sensor.camera.rgb");
  assert(cameraBpPtr != nullptr && "Carla: Can not find sensor blueprint");
  auto cameraBp = static_cast<carla::client::ActorBlueprint>(*(cameraBpPtr));
  cameraBp.SetAttribute("fov", "120");
  cameraBp.SetAttribute("image_size_x", "1280");
  cameraBp.SetAttribute("image_size_y", "720");
  cameraBp.SetAttribute("sensor_tick", "0.02"); // 50 fps
  _frameRate = 50;
  _frameSize = cv::Size(1280, 720);

  // Spawn a camera attached to the vehicle.
  auto cameraTransform = carla::geom::Transform {
      carla::geom::Location{-5.5f, 0.0f, 2.8f},   // x, y, z.
      carla::geom::Rotation{-15.0f, 0.0f, 0.0f}}; // pitch, yaw, roll.
  auto camActor = world.SpawnActor(cameraBp, cameraTransform, actor.get());
  _rgbCamera = boost::static_pointer_cast<carla::client::Sensor>(camActor);

  _rgbCamera->Listen(std::bind(&data_reader::Carla::readRgbCameraData, this, std::placeholders::_1));
}

data_reader::Carla::~Carla() {
  std::cout << "Destruct Data from Carla" << std::endl;
  _egoVehicle->Destroy();
  _rgbCamera->Destroy();
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
}
