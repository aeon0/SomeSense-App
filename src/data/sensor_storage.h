#pragma once

#include <map>
#include <vector>
#include "util/runtime_meas_service.h"
#include "camera/icam.h"
#include "rec/irec.h"
#include "frame.pb.h"


namespace data {
  class SensorStorage {
  public:
    SensorStorage(util::RuntimeMeasService& runtimeMeasService);

    void createFromConfig(const std::string filepath);
    void reset();
    void fillFrame(proto::Frame& frame);

  private:
    typedef std::map<const std::string, std::shared_ptr<ICam>> CamMap;

    // If key is left empty, a unique key will be generated
    // otherwise the user must guarantee the uniqueness of the key
    std::string addCam(std::shared_ptr<ICam> cam, std::string camKey = "");

    CamMap _cams;
    std::shared_ptr<IRec> _rec;
    util::RuntimeMeasService& _runtimeMeasService;
    unsigned int _sensorCounter; // used to create unique sensor ids

    // Rec info
    bool _isRec;
    int64_t _recLength;
  };
}