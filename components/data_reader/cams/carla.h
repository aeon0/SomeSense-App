#pragma once

#include <tuple>
#include "base_cam.h"


namespace data_reader {
  class Carla : public BaseCam {
  public:
    Carla(const std::string name, const TS& algoStartTime);

  private:
    void readData();
  };
}
