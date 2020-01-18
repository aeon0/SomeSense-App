#pragma once

#include <string>
#include "types.h"


namespace output {
  class Storage {
  public:
    void set(Frame frame);
    Frame get();
    std::string getJsonStr();

  private:
    Frame _frameData;
    std::string _frameDataJsonStr;
  };
}
