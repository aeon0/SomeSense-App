#include "output_storage.h"
#include <mutex>

std::mutex outputStateLock;

void output::OutputStorage::set(output::OutputState outputState) {
  std::lock_guard<std::mutex> lockGuard(outputStateLock);
  // TODO: Convert to json
  _outputState = outputState;
}

std::string output::OutputStorage::getJson() {
  std::lock_guard<std::mutex> lockGuard(outputStateLock);
  return _outputStateJson;
}

output::OutputState output::OutputStorage::get() {
  std::lock_guard<std::mutex> lockGuard(outputStateLock);
  return _outputState;
}
