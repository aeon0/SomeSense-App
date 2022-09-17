#pragma once

#include <tuple>
#include "base_cam.h"
#include "../sim/iscene.h"

using namespace std::chrono_literals;


namespace data_reader {
  class Carla : public BaseCam {
  public:
    Carla(const std::string name, const TS& algoStartTime);
    void start() override;

  private:
    void readRgbCameraData(carla::SharedPtr<carla::sensor::SensorData> sensorData);

    std::shared_ptr<sim::IScene> _scene;
    boost::shared_ptr<carla::client::Sensor> _rgbCam;
  };
}
