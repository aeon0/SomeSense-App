#include <string>
#include <chrono>
#include <iostream>
#include <iomanip>
#include "utilities/json.hpp"


namespace frame {
  class RuntimeMeasService {
  public:
    RuntimeMeasService();

    void startMeas(std::string name);
    void endMeas(std::string name);
    void printToConsole();
    nlohmann::json serializeMeas();

    void reset() { _meas.clear(); }
    void setStartTime(const std::chrono::time_point<std::chrono::high_resolution_clock> algoStartTime) {
      _algoStartTime = algoStartTime;
    }

  private:
    struct RuntimeMeas {
      std::chrono::_V2::system_clock::time_point startTime;
      std::chrono::_V2::system_clock::time_point endTime;
      std::chrono::duration<double, std::milli> duration;
      bool running;
    };

    std::map<std::string, RuntimeMeas> _meas;
    std::chrono::time_point<std::chrono::high_resolution_clock> _algoStartTime;
  };
}
