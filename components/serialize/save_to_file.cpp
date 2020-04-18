#include "save_to_file.h"
#include <chrono>
#include <iostream>
#include <thread>
#include <fcntl.h>
#include <unistd.h>


serialize::SaveToFile::SaveToFile(const std::string storageBasePath, serialize::AppState& appState) :
  _storageBasePath(storageBasePath), _appState(appState), _lastSavedTs(-1), _isStoring(false) {}

void serialize::SaveToFile::handleRequest(const std::string& requestType, const nlohmann::json& requestData, nlohmann::json& responseData) {
  if (requestData["type"] == "client.start_storing") {
    std::cout << "Start save to file" << std::endl;
    start();
  }
  else if (requestData["type"] == "client.stop_storing") {
    std::cout << "Stop save to file" << std::endl;
    stop();
  }
}

std::string serialize::SaveToFile::formatTimePoint(std::chrono::system_clock::time_point point) {
  static_assert(std::chrono::system_clock::time_point::period::den == 1000000000 && std::chrono::system_clock::time_point::period::num == 1);
  std::string out(29, '0');
  char* buf = &out[0];
  std::time_t now_c = std::chrono::system_clock::to_time_t(point);
  std::strftime(buf, 21, "%Y-%m-%dT%H:%M:%S.", std::localtime(&now_c));
  sprintf(buf+20, "%09ld", point.time_since_epoch().count() % 1000000000);
  return out;
}

void serialize::SaveToFile::stop() {
  if (_isStoring) {
    std::lock_guard<std::mutex> lockGuard(_storageServiceMtx);
    _isStoring = false;
    _appState.setSaveToFileState(false);
  }
}

void serialize::SaveToFile::start() {
  if (!_isStoring) {
    std::lock_guard<std::mutex> lockGuard(_storageServiceMtx);
    // Create folder to store rec into
    std::ostringstream folderName;
    _currFilePath = _storageBasePath + "/rec_" + formatTimePoint(std::chrono::system_clock::now()) + ".capnp.bin";
    _lastSavedTs = -1;
    _isStoring = true;
    _appState.setSaveToFileState(true);
    
    std::thread dataStorageThread(&serialize::SaveToFile::run, this);
    dataStorageThread.detach();
  }
}

void serialize::SaveToFile::run() {
  while (_isStoring) {
    {
      std::lock_guard<std::mutex> lockGuard(_storageServiceMtx);
      int64_t currAlgoTs = _appState.getAlgoTs();
      if (currAlgoTs > _lastSavedTs) {
        _lastSavedTs = currAlgoTs;
        saveFrame();
      }
    }
    // Polling every 5 ms to check if there is new data
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
}

void serialize::SaveToFile::saveFrame() {
  if (_isStoring) {
    int fd = open(_currFilePath.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0755);
    if (_appState.writeToFile(fd)) {
      close(fd);
    }
  }
}
