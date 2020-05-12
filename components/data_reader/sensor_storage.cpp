#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "utilities/json.hpp"

#include <iostream>
#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#include "serialize/frame.capnp.h"
#include "sensor_storage.h"
#include "cams/rec_cam.h"
#include "cams/usb_cam.h"
#include "cams/csi_cam.h"
#ifdef BUILD_SIM // Set by CMake
#include "cams/carla.h"
#endif


data_reader::SensorStorage::SensorStorage(com_out::IRequestHandler& requestHandler, const TS& algoStartTime) :
  _requestHandler(requestHandler), _algoStartTime(algoStartTime), _sensorCounter(0) {}

void data_reader::SensorStorage::createFromConfig(const std::string& filepath, serialize::AppState& appState) {
  std::ifstream ifs(filepath);
  if (!ifs.good()) {
    throw std::runtime_error("Could not open sensor config file: " + filepath);
  }

  nlohmann::json timestampsJson;
  auto jsonSensorConfig = nlohmann::json::parse(ifs);

  if (jsonSensorConfig.contains("rec")) {
    std::cout << "** Load recording **" << std::endl;
    auto filePath = jsonSensorConfig["rec"].get<std::string>();
    int fd = open(filePath.c_str(), O_RDONLY);
    struct stat s;
    fstat(fd, &s);
    double fSizeGb = static_cast<double>(s.st_size) * 0.000000001f;
    std::cout << "File Path: " << filePath << std::endl;
    std::cout << "File Size: " << fSizeGb <<  " [GB]" << std::endl;

    kj::FdInputStream fdStream(fd);
    kj::BufferedInputStreamWrapper bufferedStream(fdStream);
    kj::ArrayPtr<const kj::byte> framePtr = bufferedStream.tryGetReadBuffer();
    if (framePtr != nullptr) {
      capnp::PackedMessageReader message(bufferedStream);
      auto frame = message.getRoot<CapnpOutput::Frame>();
      auto camSensors = frame.getCamSensors();
      for (int i = 0; i < camSensors.size(); ++i) {
        std::string key = camSensors[i].getKey();
        auto recCam = std::make_shared<RecCam>(
          key,
          camSensors[i].getFovHorizontal(),
          camSensors[i].getImg().getWidth(),
          camSensors[i].getImg().getHeight(),
          appState,
          filePath
        );
        _requestHandler.registerRequestListener(recCam);
        addCam(recCam);
      }
    }
    close(fd);
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
        throw std::runtime_error("Type " + typeName + " is not supported!");
      }
    }
  }

  // Start all created cameras
  for (auto const [key, cam]: _cams) {
    cam->start();
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
