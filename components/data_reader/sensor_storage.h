#pragma once

#include <map>
#include <vector>
#include "types.h"
#include "cams/icam.h"
#include "com_out/irequest_handler.h"
#include "serialize/app_state.h"


namespace data_reader {
  class SensorStorage {
  public:
    SensorStorage(com_out::IRequestHandler& requestHandler, const TS& algoStartTime);

    typedef std::map<const std::string, std::shared_ptr<ICam>> CamMap;

    void createFromConfig(const std::string& filepath, serialize::AppState& appState);
    // If key is left empty, a unique key will be generated
    // otherwise the user must guarantee the uniqueness of the key
    std::string addCam(std::shared_ptr<ICam> cam, std::string camKey = "");

    const CamMap& getCams() const { return _cams; };

  private:
    CamMap _cams;
    unsigned int _sensorCounter; // used to create unique sensor ids
    const TS& _algoStartTime;

    com_out::IRequestHandler& _requestHandler;
  };
}
