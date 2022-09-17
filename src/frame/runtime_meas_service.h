#pragma once

#include <string>
#include <chrono>
#include <map>
#include "types.h"
#include "serialize/frame.capnp.h"


namespace frame {
  class RuntimeMeasService {
  public:
    struct RuntimeMeas {
      std::chrono::_V2::system_clock::time_point startTime;
      std::chrono::_V2::system_clock::time_point endTime;
      std::chrono::duration<double, std::milli> duration;
      bool running;
    };

    RuntimeMeasService(const TS& algoStartTime);

    void startMeas(std::string name);
    void endMeas(std::string name);
    void printToConsole();

    void reset() { _meas.clear(); }

    void serialize(CapnpOutput::Frame::Builder& builder);

  private:
    std::map<std::string, RuntimeMeas> _meas;
    const TS& _algoStartTime;
  };
}
