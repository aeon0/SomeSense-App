#pragma once

#include <string>
#include "types.h"
#include "utilities/json.hpp"


namespace output {
  class Storage {
  public:
    void set(Frame frame);

    Frame get() const;
    nlohmann::json getJson() const;
    int64_t getAlgoTs() const;

  private:
    Frame _frameData;
    nlohmann::json _frameDataJson;
  };
}
