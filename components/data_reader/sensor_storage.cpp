#include <fstream>
#include "utilities/json.hpp"

#include <iostream>
#include "sensor_storage.h"
#include "cams/video_cam.h"


void data_reader::SensorStorage::initFromConfig(const std::string& filepath) {
  std::ifstream ifs(filepath);
  if (!ifs.good()) {
    throw std::runtime_error("Could not open sensor config file:" + filepath );
  }

  nlohmann::json jsonSensorConfig = nlohmann::json::parse(ifs);
  for(const auto it: jsonSensorConfig["cams"]) {
    if (it["type"] == "video") {
      std::cout << it << std::endl;
      const VideoCam videoCam(it["filepath"]);
      addCam(videoCam);
    }
  }
}

std::string data_reader::SensorStorage::addCam(const ICam& cam) {
  std::string camId = std::to_string(_sensorCounter++);
  _cams.insert({camId, cam});
  return camId;
}
