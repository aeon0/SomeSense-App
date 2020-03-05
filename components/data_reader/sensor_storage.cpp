#include <fstream>
#include "utilities/json.hpp"

#include <iostream>
#include "sensor_storage.h"
#include "cams/video_cam.h"
#include "cams/usb_cam.h"
#include "cams/csi_cam.h"


data_reader::SensorStorage::SensorStorage(com_out::IRequestHandler& requestHandler, const TS& algoStartTime, output::Storage& outputStorage) :
  _outputStorage(outputStorage), _requestHandler(requestHandler), _algoStartTime(algoStartTime), _sensorCounter(0) {}

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
    const auto camName = it["name"].get<std::string>();
    if (typeName == "video") {
      const std::string filePath = jsonSensorConfig["basepath"].get<std::string>() + "/" + camName + ".mp4";
      if (timestampsJson.contains(camName)) {
        auto timestamps = timestampsJson[camName].get<std::vector<int64>>();
        auto videoCam = std::make_shared<VideoCam>(camName, _algoStartTime, _outputStorage, filePath, timestamps);
        _requestHandler.registerRequestListener(videoCam);
        addCam(videoCam, camName);
      }
      else {
        auto videoCam = std::make_shared<VideoCam>(camName, _algoStartTime, _outputStorage, filePath);
        _requestHandler.registerRequestListener(videoCam);
        addCam(videoCam, camName);
      }
    }
    else if (typeName == "usb") {
      auto usbCam = std::make_shared<UsbCam>(camName, _algoStartTime, it["device_idx"].get<int>());
      addCam(usbCam);
    }
    else if (typeName == "csi") {
      auto captureWidth = it["capture_width"].get<int>();
      auto captureHeight = it["capture_height"].get<int>();
      auto flipMethod = it["flip_method"].get<int>();
      auto frameRate = it["frame_rate"].get<int>();

      auto csiCam = std::make_shared<CsiCam>(camName, _algoStartTime, captureWidth, captureHeight, frameRate, flipMethod);
      addCam(csiCam);
    }
    else {
      throw std::runtime_error("Type " + typeName + " is not supported yet!");
    }
  }
}

std::string data_reader::SensorStorage::addCam(std::shared_ptr<ICam> cam, std::string camKey) {
  if (camKey == "") {
    camKey = cam->getName() + "_" + std::to_string(_sensorCounter);
  }
  _cams.insert({camKey, cam});
  _sensorCounter++;
  return camKey;
}
