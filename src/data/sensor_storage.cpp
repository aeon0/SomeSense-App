#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>

#include "util/json.hpp"
#include "frame.pb.h"
#include "sensor_storage.h"
#include "camera/video_cam.h"


data::SensorStorage::SensorStorage(util::RuntimeMeasService& runtimeMeasService) :
  _runtimeMeasService(runtimeMeasService), _sensorCounter(0) {}

void data::SensorStorage::fillFrame(proto::Frame& frame) {
  _runtimeMeasService.startMeas("get_cam_data");
  for (auto const [key, cam]: _cams) {
    auto camSensor = frame.mutable_camsensors()->Add();
    camSensor->set_key(key);
    cam->fillCamData(*camSensor);
  }
  _runtimeMeasService.endMeas("get_cam_data");
}

void data::SensorStorage::createFromConfig(const std::string filepath) {
  std::ifstream ifs(filepath);
  if (!ifs.good()) {
    throw std::runtime_error("Could not open sensor config file: " + filepath);
  }

  nlohmann::json timestampsJson;
  auto jsonSensorConfig = nlohmann::json::parse(ifs);

  if (jsonSensorConfig.contains("video_path")) {
    std::cout << "** Load video from mp4 **" << std::endl;
    auto filePath = jsonSensorConfig["video_path"].get<std::string>();
    auto videoCam = std::make_shared<VideoCam>("VideoCam", filePath);
    addCam(videoCam);
  }
  else {
    throw std::runtime_error("Could not figure out what to do with config: " + filepath);
  }
}

std::string data::SensorStorage::addCam(std::shared_ptr<ICam> cam, std::string camKey) {
  if (camKey == "") {
    camKey = cam->getName() + "_" + std::to_string(_sensorCounter);
  }
  _cams.insert({camKey, cam});
  _sensorCounter++;
  return camKey;
}
