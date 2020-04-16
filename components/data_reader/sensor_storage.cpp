#include <fstream>
#include "utilities/json.hpp"

#include <iostream>
#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#include "serialize/frame.capnp.h"
#include <fcntl.h>
#include <unistd.h>
#include "sensor_storage.h"
#include "cams/rec_cam.h"
#include "cams/usb_cam.h"
#include "cams/csi_cam.h"
#ifdef BUILD_SIM // Set by CMake
#include "cams/carla.h"
#endif


data_reader::SensorStorage::SensorStorage(com_out::IRequestHandler& requestHandler, const TS& algoStartTime) :
  _requestHandler(requestHandler), _algoStartTime(algoStartTime), _sensorCounter(0) {}

void data_reader::SensorStorage::initFromConfig(const std::string& filepath) {
  std::ifstream ifs(filepath);
  if (!ifs.good()) {
    throw std::runtime_error("Could not open sensor config file: " + filepath);
  }

  nlohmann::json timestampsJson;
  auto jsonSensorConfig = nlohmann::json::parse(ifs);

  if (jsonSensorConfig.contains("rec")) {
    std::cout << "** Load recording **" << std::endl;
    // TODO: Maybe put this in a more dedicated place to reading rec files
    const auto recFilePath = jsonSensorConfig["rec"].get<std::string>();
    int fd = open(recFilePath.c_str(), O_RDONLY);
    kj::FdInputStream fdStream(fd);
    kj::BufferedInputStreamWrapper bufferedStream(fdStream);
    std::vector<std::string> recCamKeys;
    while (bufferedStream.tryGetReadBuffer() != nullptr) {
      capnp::PackedMessageReader message(bufferedStream);
      auto frame = message.getRoot<CapnpOutput::Frame>();
      auto camSensors = frame.getCamSensors();
      for (int i = 0; i < camSensors.size(); ++i) {
        auto recCam = std::make_shared<RecCam>(
          camSensors[i].getKey(),
          camSensors[i].getFovHorizontal(),
          camSensors[i].getImg().getWidth(),
          camSensors[i].getImg().getHeight(),
          recFilePath
        );
        recCamKeys.push_back(addCam(recCam, camSensors[i].getKey()));
      }
      break;
    }
    close(fd);
    for (auto key: recCamKeys) {
      _cams[key]->start();
    }
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

        auto usbCam = std::make_shared<UsbCam>(camName, _algoStartTime, device_idx, captureWidth, captureHeight, horizontalFov);
        addCam(usbCam);
        usbCam->start();
      }
      else if (typeName == "csi") {
        auto captureWidth = it["capture_width"].get<int>();
        auto captureHeight = it["capture_height"].get<int>();
        auto flipMethod = it["flip_method"].get<int>();
        auto frameRate = it["frame_rate"].get<int>();
        auto horizontalFov = it["horizontal_fov"].get<double>() * (M_PI / 180.0);

        auto csiCam = std::make_shared<CsiCam>(camName, _algoStartTime, captureWidth, captureHeight, frameRate, flipMethod, horizontalFov);
        addCam(csiCam);
        csiCam->start();
      }
#ifdef BUILD_SIM
      else if (typeName == "carla") {
        auto carlaCam = std::make_shared<Carla>(camName, _algoStartTime);
        addCam(carlaCam);
        carlaCam->start();
      }
#endif
      else {
        throw std::runtime_error("Type " + typeName + " is not supported!");
      }
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
