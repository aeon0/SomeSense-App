#include "sensor_storage.h"

std::string data_reader::SensorStorage::addCam(const ICam& cam) {
  std::string camId = std::to_string(_sensorCounter++);
  _cams.insert({camId, cam});
  return camId;
}
