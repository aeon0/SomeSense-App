#include "runtime_meas_service.h"
#include <iostream>
#include <iomanip>
#include "util/time.h"


util::RuntimeMeasService::RuntimeMeasService() {}

void util::RuntimeMeasService::startMeas(std::string name) {
  _meas.erase(name); // In case the key does not exist at all, this does nothing
  RuntimeMeas meas;
  meas.running = true;
  meas.startTime = std::chrono::high_resolution_clock::now();
  _meas.insert({name, meas});
}

void util::RuntimeMeasService::endMeas(std::string name) {
  if(_meas.find(name) == _meas.end()) {
    std::cout << "WARNING: Can not end runtime meas: " << name << " does not exist!" << std::endl;
  }
  else {
    _meas.at(name).running = false;
    _meas.at(name).endTime = std::chrono::high_resolution_clock::now();
    _meas.at(name).duration = std::chrono::duration<double, std::milli>(_meas.at(name).endTime - _meas.at(name).startTime);
  }
}

void util::RuntimeMeasService::printToConsole() {
  std::cout << std::endl;
  for(auto& [key, value]: _meas) {
    if(!value.running) {
      const std::string keyStr = key + ":";
      std::cout << std::left << std::setw(30) << std::setfill(' ') << keyStr <<
        std::right << std::setw(10) << std::setfill(' ') << std::fixed << std::setprecision(4) << value.duration.count() << " ms" << std::endl;
    }
  }
}

void util::RuntimeMeasService::serialize(proto::Frame& data, const util::TS& appStartTime) {
  int i = 0;
  for (auto [key, val]: _meas) {
    auto pm = data.mutable_runtimemeas()->Add();
    pm->set_name(key);
    pm->set_duration(val.duration.count());
    pm->set_absstart(util::timepointToInt64(val.startTime));
    pm->set_relstart(util::calcDurationInInt64(val.startTime, appStartTime));
  }
}
