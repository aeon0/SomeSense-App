#include <string>
#include <chrono>
#include <iostream>
#include <map>
#include <iomanip>
#include "output/types.h"
#include "types.h"


namespace frame {
  class RuntimeMeasService {
  public:
    RuntimeMeasService(const TS& algoStartTime);

    void startMeas(std::string name);
    void endMeas(std::string name);
    void printToConsole();
    std::vector<output::RuntimeMeas> serializeMeas();

    void reset() { _meas.clear(); }

  private:
    struct RuntimeMeas {
      std::chrono::_V2::system_clock::time_point startTime;
      std::chrono::_V2::system_clock::time_point endTime;
      std::chrono::duration<double, std::milli> duration;
      bool running;
    };

    std::map<std::string, RuntimeMeas> _meas;
    const TS& _algoStartTime;
  };
}
