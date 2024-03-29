#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>

#include "util/json.hpp"
#include "frame.pb.h"
#include "sensor_storage.h"
#include "camera/usb_cam.h"
#include "camera/csi_cam.h"
#include "rec/video_rec.h"
#include "rec/pb_rec.h"


data::SensorStorage::SensorStorage(util::RuntimeMeasService& runtimeMeasService) :
  _runtimeMeasService(runtimeMeasService), _sensorCounter(0), _rec(nullptr), _isRec(false), _recLength(0) {}

void data::SensorStorage::fillFrame(proto::Frame& frame, const util::TS& appStartTime) {
  _runtimeMeasService.startMeas("get_sensor_data");
  if (_isRec) {
    if (_rec != nullptr) {
      _rec->fillFrame(frame);
    }
  }
  else {
    for (auto const [key, cam]: _cams) {
      auto camSensor = frame.mutable_camsensors()->Add();
      camSensor->set_key(key);
      cam->fillCamData(*camSensor, appStartTime);
    }
  }
  _runtimeMeasService.endMeas("get_sensor_data");
}

void data::SensorStorage::reset() {
  if (_rec != nullptr) {
    _rec.reset();
  }
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
    _rec = std::make_shared<VideoRec>(filePath);
    _isRec = true;
    _recLength = _rec->getRecLength();
  }
  else if (jsonSensorConfig.contains("pb_bin") && jsonSensorConfig.contains("meta")) {
    std::cout << "** Load data from protobuf **" << std::endl;
    auto binPath = jsonSensorConfig["pb_bin"].get<std::string>();
    auto metaPath = jsonSensorConfig["meta"].get<std::string>();
    _rec = std::make_shared<PbRec>(binPath, metaPath);
    _isRec = true;
    _recLength = _rec->getRecLength();
  }
  else {
    for (const auto it: jsonSensorConfig["cams"]) {
      const auto typeName = it["type"].get<std::string>();
      const auto camName = it["name"].get<std::string>();
      if (typeName == "usb") {
        auto captureWidth = it["capture_width"].get<int>();
        auto captureHeight = it["capture_height"].get<int>();
        auto device_idx = it["device_idx"].get<int>();
        auto horizontalFov = it["horizontal_fov"].get<double>() * (M_PI / 180.0);

        auto usbCam = std::make_shared<UsbCam>(camName, device_idx, captureWidth, captureHeight, horizontalFov);
        addCam(usbCam);
      }
      else if (typeName == "csi") {
          auto captureWidth = it["capture_width"].get<int>();
          auto captureHeight = it["capture_height"].get<int>();
          auto frameRate = it["frame_rate"].get<double>();
          auto flipMethod = it["flip_method"].get<int>();
          auto horizontalFov = it["horizontal_fov"].get<double>() * (M_PI / 180.0);

          auto csiCam = std::make_shared<CsiCam>(camName, captureWidth, captureHeight, horizontalFov, frameRate, flipMethod);
          addCam(csiCam);
      }
      else {
        throw std::runtime_error("Camera Type " + typeName + " is not supported!");
      }
    }
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
