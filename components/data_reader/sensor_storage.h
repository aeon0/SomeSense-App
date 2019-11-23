#ifndef sensor_storage_h
#define sensor_storage_h

#include <map>
#include "cams/icam.h"

namespace data_reader {
  class SensorStorage {
  public:
    typedef std::map<const std::string, const ICam&> CamMap;

    void initFromConfig(const std::string& filepath);
    std::string addCam(const ICam& cam);

    const CamMap getCams() const { return _cams; };

  private:
    CamMap _cams;
    unsigned int _sensorCounter; // used to create unique sensor ids
  };
}

#endif
