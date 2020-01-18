#pragma once

#include <string>
#include <vector>
#include "types.h"
#include "utilities/json.hpp"


namespace output {
  class Storage {
  public:
    typedef std::map<const std::string, CamImg> CamImgMap;

    // Algo data + Sensor meta data
    void set(Frame frame);
    Frame get() const;
    nlohmann::json getJson() const;
    int64_t getAlgoTs() const;

    // Cam sensor raw output data
    void setCamImg(std::string key, CamImg data);
    CamImgMap getCamImgs() const;

  private:
    Frame _frameData;
    nlohmann::json _frameDataJson;

    CamImgMap _camImgs;
  };
}
