#ifndef sensor_storage_h
#define sensor_storage_h

#include <map>
#include <vector>
#include "cams/icam.h"

namespace data_reader {
  class SensorStorage {
  public:
    SensorStorage();

    typedef std::map<const std::string, std::unique_ptr<ICam>> CamMap;

    void initFromConfig(const std::string& filepath);
    std::string addCam(std::unique_ptr<ICam>& cam);

    const CamMap& getCams() const { return _cams; };

  private:
    CamMap _cams;
    unsigned int _sensorCounter; // used to create unique sensor ids
  };
}

#endif
