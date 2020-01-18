#include "output_storage.h"
#include <mutex>

std::mutex outputStateLock;

void output::OutputStorage::set(output::OutputState outputState) {
  std::lock_guard<std::mutex> lockGuard(outputStateLock);
  // nlohmann::json jsonOutput = outputState;
  // _outputStateJsonStr = jsonOutput.dump();
  _outputState = outputState;
}

std::string output::OutputStorage::getJsonStr() {
  std::lock_guard<std::mutex> lockGuard(outputStateLock);
  return _outputStateJsonStr;
}

output::OutputState output::OutputStorage::get() {
  std::lock_guard<std::mutex> lockGuard(outputStateLock);
  return _outputState;
}
