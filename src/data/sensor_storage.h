#pragma once

#include <map>
#include <vector>
#include "util/runtime_meas_service.h"
#include "camera/icam.h"
#include "frame.pb.h"


namespace data {
  class SensorStorage {
  public:
    SensorStorage(util::RuntimeMeasService& runtimeMeasService);

    void createFromConfig(const std::string filepath);
    void fillFrame(proto::Frame& frame);

  private:
    typedef std::map<const std::string, std::shared_ptr<ICam>> CamMap;

    // If key is left empty, a unique key will be generated
    // otherwise the user must guarantee the uniqueness of the key
    std::string addCam(std::shared_ptr<ICam> cam, std::string camKey = "");

    CamMap _cams;
    util::RuntimeMeasService& _runtimeMeasService;
    unsigned int _sensorCounter; // used to create unique sensor ids
  };
}
