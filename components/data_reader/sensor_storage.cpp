#include <fstream>
#include "utilities/json.hpp"

#include <iostream>
#include "sensor_storage.h"
#include "cams/video_cam.h"
#include "cams/usb_cam.h"

data_reader::SensorStorage::SensorStorage() : _sensorCounter(0) {}

void data_reader::SensorStorage::initFromConfig(const std::string& filepath) {
  std::ifstream ifs(filepath);
  if (!ifs.good()) {
    throw std::runtime_error("Could not open sensor config file: " + filepath);
  }

  nlohmann::json jsonSensorConfig = nlohmann::json::parse(ifs);
  for (const auto it: jsonSensorConfig["cams"]) {
    const std::string typeName = static_cast<std::string>(it["type"]);
    if (typeName == "video") {
      std::unique_ptr<ICam> videoCam(new VideoCam(it["filepath"], it["name"]));
      addCam(videoCam);
    }
    else if (typeName == "usb") {
      std::unique_ptr<ICam> usbCam(new UsbCam(static_cast<int>(it["deviceIdx"]), it["name"]));
      addCam(usbCam);
    }
    else {
      throw std::runtime_error("Type " + typeName + " is not supported yet!");
    }
  }
}

std::string data_reader::SensorStorage::addCam(std::unique_ptr<ICam>& cam) {
  const std::string camKey = cam->getName() + "_" + std::to_string(_sensorCounter++);
  _cams.insert({camKey, std::move(cam)});
  return camKey;
}
