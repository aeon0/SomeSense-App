#ifndef sensor_storage_h
#define sensor_storage_h

#include "cams/icam.h"

namespace data_reader {
  class SensorStorage {
  public:
    std::string addCam(const ICam& cam);

  private:
    std::map<std::string, const ICam&> _cams;
  };
}

#endif
