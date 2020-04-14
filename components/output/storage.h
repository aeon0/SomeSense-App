#pragma once

#include <string>
#include <vector>
#include <capnp/message.h>
#include "output/output_types.capnp.h"
#include "types.h"
#include "utilities/json.hpp"


namespace output {
  class Storage {
  public:
    Storage();

    typedef std::map<const std::string, CamImg> CamImgMap;

    // Algo data + Sensor meta data
    void set(Frame frame);
    void set(CtrlData ctrlData);
    Frame getFrame() const;
    CtrlData getCtrlData() const;

    nlohmann::json getFrameJson() const;
    nlohmann::json getCtrlDataJson() const;

    int64_t getAlgoTs() const;

    // Cam sensor raw output data
    // Dont forget to clone the cv::Mat of data!
    void setCamImg(std::string key, CamImg data);
    // Output by reference because it needs to be cloned during an active lock
    void getCamImgs(CamImgMap& camImgMap) const;

    void set(CapnpOutput::Frame frame);

  private:
    Frame _frameData;
    CtrlData _ctrlData;
    nlohmann::json _frameDataJson;
    nlohmann::json _ctrlDataJson;
    CamImgMap _camImgs;

    capnp::MallocMessageBuilder _messageBuilder;
    CapnpOutput::Frame::Builder _currFrame;
  };
}
