#pragma once
#include "own_capnp.h"
#include "../sensor_storage.h"


namespace data_reader {
  namespace rec {
    // Takes file of packed capnp frame data and creates the sensors accordingly
    void createFromFile(const std::string filePath, SensorStorage& storage, com_out::IRequestHandler& requestHandler);
  }
}
