#pragma once

#include <string>
#include "output_types.h"


namespace output {
  class OutputStorage {
  public:
    void set(OutputState outputState);
    OutputState get();
    std::string getJsonStr();

  private:
    OutputState _outputState;
    std::string _outputStateJsonStr;
  };
}
