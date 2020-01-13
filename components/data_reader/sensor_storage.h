#pragma once

#include <map>
#include <vector>
#include "types.h"
#include "cams/icam.h"

namespace data_reader {
  class SensorStorage {
  public:
    SensorStorage(const TS& algoStartTime);

    typedef std::map<const std::string, std::unique_ptr<ICam>> CamMap;

    void initFromConfig(const std::string& filepath);
    // If key is left empty, a unique key will be generated
    // otherwise the user must guarantee the uniqueness of the key
    std::string addCam(std::unique_ptr<ICam>& cam, std::string camKey = "");

    const CamMap& getCams() const { return _cams; };

  private:
    CamMap _cams;
    unsigned int _sensorCounter; // used to create unique sensor ids
    const TS& _algoStartTime;
  };
}
