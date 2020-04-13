#include <fstream>
#include "utilities/json.hpp"

#include <iostream>
#include "sensor_storage.h"
#include "cams/video_cam.h"
#include "cams/usb_cam.h"
#include "cams/csi_cam.h"
#ifdef BUILD_SIM // Set by CMake
#include "cams/carla.h"
#endif


data_reader::SensorStorage::SensorStorage(com_out::IRequestHandler& requestHandler, const TS& algoStartTime, output::Storage& outputStorage) :
  _outputStorage(outputStorage), _requestHandler(requestHandler), _algoStartTime(algoStartTime), _sensorCounter(0) {}

void data_reader::SensorStorage::initFromConfig(const std::string& filepath) {
  std::ifstream ifs(filepath);
  if (!ifs.good()) {
    throw std::runtime_error("Could not open sensor config file: " + filepath);
  }

  nlohmann::json timestampsJson;
  auto jsonSensorConfig = nlohmann::json::parse(ifs);

  for (const auto it: jsonSensorConfig["cams"]) {
    const auto typeName = it["type"].get<std::string>();
    const auto camName = it["name"].get<std::string>();
    if (typeName == "video") {
      if (it.contains("timestamps")) {
        // Load timestamps file
        std::ifstream ifsTs(it["basepath"].get<std::string>() + "/" + it["timestamps"].get<std::string>());
        if (!ifsTs.good()) {
          throw std::runtime_error("Could not open timestamp file, please check its path in the json config");
        }
        timestampsJson = nlohmann::json::parse(ifsTs);
      }

      auto horizontalFov = it["horizontal_fov"].get<double>() * (M_PI / 180.0); // convert from deg to rad
      const std::string filePath = it["basepath"].get<std::string>() + "/" + camName + ".mp4";
      if (timestampsJson.contains(camName)) {
        auto timestamps = timestampsJson[camName].get<std::vector<int64>>();
        auto videoCam = std::make_shared<VideoCam>(camName, _algoStartTime, _outputStorage, filePath, horizontalFov, timestamps);
        _requestHandler.registerRequestListener(videoCam);
        addCam(videoCam, camName);
      }
      else {
        auto videoCam = std::make_shared<VideoCam>(camName, _algoStartTime, _outputStorage, filePath, horizontalFov);
        _requestHandler.registerRequestListener(videoCam);
        addCam(videoCam, camName);
      }
    }
    else if (typeName == "usb") {
      auto captureWidth = it["capture_width"].get<int>();
      auto captureHeight = it["capture_height"].get<int>();
      auto device_idx = it["device_idx"].get<int>();
      auto horizontalFov = it["horizontal_fov"].get<double>() * (M_PI / 180.0);

      auto usbCam = std::make_shared<UsbCam>(camName, _algoStartTime, device_idx, captureWidth, captureHeight, horizontalFov);
      addCam(usbCam);
    }
    else if (typeName == "csi") {
      auto captureWidth = it["capture_width"].get<int>();
      auto captureHeight = it["capture_height"].get<int>();
      auto flipMethod = it["flip_method"].get<int>();
      auto frameRate = it["frame_rate"].get<int>();
      auto horizontalFov = it["horizontal_fov"].get<double>() * (M_PI / 180.0);

      auto csiCam = std::make_shared<CsiCam>(camName, _algoStartTime, captureWidth, captureHeight, frameRate, flipMethod, horizontalFov);
      addCam(csiCam);
    }
#ifdef BUILD_SIM
    else if (typeName == "carla") {
      auto carlaCam = std::make_shared<Carla>(camName, _algoStartTime);
      addCam(carlaCam);
    }
#endif
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
