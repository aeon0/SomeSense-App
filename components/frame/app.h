#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "data_reader/sensor_storage.h"
#include "com_out/unix_server.h"
#include "object_detection/detector.h"


namespace frame {
  class App {
  public:
    void init(const std::string& sensorConfigPath);
    void start();

  private:
    data_reader::SensorStorage _sensorStorage;
    com_out::UnixServer _server;
    object_detection::Detector _detector;
  };
}
