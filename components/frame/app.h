#ifndef app_h
#define app_h

#include "data_reader/sensor_storage.h"

namespace frame {
  class App {
  public:
    void init(const std::string& sensorConfigPath);
    void start();

  private:
    data_reader::SensorStorage _sensorStorage;
  };
}

#endif
