#pragma once

#include <tuple>
#include "base_cam.h"
#include <carla/client/Sensor.h>
#include <carla/client/ActorBlueprint.h>
#include <carla/client/BlueprintLibrary.h>
#include <carla/client/Client.h>
#include <carla/client/Map.h>
#include <carla/client/World.h>
#include <carla/geom/Transform.h>
#include <carla/image/ImageIO.h>
#include <carla/image/ImageView.h>
#include <carla/sensor/data/Image.h>


using namespace std::chrono_literals;

namespace data_reader {
  class Carla : public BaseCam {
  public:
    Carla(const std::string name, const TS& algoStartTime);
    ~Carla();

  private:
    void readRgbCameraData(carla::SharedPtr<carla::sensor::SensorData> sensorData);

    boost::shared_ptr<carla::client::Sensor> _rgbCamera;
    boost::shared_ptr<carla::client::Vehicle> _egoVehicle;
  };
}
