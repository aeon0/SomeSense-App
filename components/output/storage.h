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
    // Dont forget to clone the cv::Mat of data!
    void setCamImg(std::string key, CamImg data);
    // Output by reference because it needs to be cloned during an active lock
    void getCamImgs(CamImgMap& camImgMap) const;

  private:
    Frame _frameData;
    nlohmann::json _frameDataJson;

    CamImgMap _camImgs;
  };
}
