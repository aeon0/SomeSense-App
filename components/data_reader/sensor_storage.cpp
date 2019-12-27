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

  nlohmann::json timestampsJson;
  auto jsonSensorConfig = nlohmann::json::parse(ifs);
  if (jsonSensorConfig.contains("timestamps")) {
    // Load timestamps file
    std::ifstream ifsTs(jsonSensorConfig["basepath"].get<std::string>() + "/" + jsonSensorConfig["timestamps"].get<std::string>());
    if (!ifsTs.good()) {
      throw std::runtime_error("Could not open timestamp file, please check its path in the json config");
    }
    timestampsJson = nlohmann::json::parse(ifsTs);
  }
  for (const auto it: jsonSensorConfig["cams"]) {
    const auto typeName = it["type"].get<std::string>();
    if (typeName == "video") {
      const std::string sensorKey = it["name"];
      const std::string filePath = jsonSensorConfig["basepath"].get<std::string>() + "/" + sensorKey + ".mp4";
      if (timestampsJson.contains(sensorKey)) {
        auto timestamps = timestampsJson[sensorKey].get<std::vector<int64>>();
        std::unique_ptr<ICam> videoCam(new VideoCam(filePath, sensorKey, timestamps));
        addCam(videoCam);
      }
      else {
        std::unique_ptr<ICam> videoCam(new VideoCam(filePath, sensorKey));
        addCam(videoCam);
      }
    }
    else if (typeName == "usb") {
      std::unique_ptr<ICam> usbCam(new UsbCam(it["deviceIdx"].get<int>(), it["name"]));
      addCam(usbCam);
    }
    else {
      throw std::runtime_error("Type " + typeName + " is not supported yet!");
    }
  }
}

std::string data_reader::SensorStorage::addCam(std::unique_ptr<ICam>& cam, std::string camKey) {
  if (camKey != "") {
    camKey = cam->getName() + "_" + std::to_string(_sensorCounter);
  }
  _cams.insert({camKey, std::move(cam)});
  _sensorCounter++;
  return camKey;
}
