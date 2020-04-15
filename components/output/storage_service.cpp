#include "storage_service.h"
#include <chrono>
#include <iostream>
#include <thread>
#include <fcntl.h>
#include <unistd.h>


output::StorageService::StorageService(const std::string storageBasePath, output::Storage& outputStorage) :
  _storageBasePath(storageBasePath), _outputStorage(outputStorage), _lastSavedTs(-1), _isStoring(false) {}

void output::StorageService::handleRequest(const std::string& requestType, const nlohmann::json& requestData, nlohmann::json& responseData) {
  if (requestData["type"] == "client.start_storing") {
    std::cout << "Start Storing" << std::endl;
    start();
  }
  else if (requestData["type"] == "client.stop_storing") {
    std::cout << "Stop Storing" << std::endl;
    stop();
  }
}

std::string output::StorageService::formatTimePoint(std::chrono::system_clock::time_point point) {
  static_assert(std::chrono::system_clock::time_point::period::den == 1000000000 && std::chrono::system_clock::time_point::period::num == 1);
  std::string out(29, '0');
  char* buf = &out[0];
  std::time_t now_c = std::chrono::system_clock::to_time_t(point);
  std::strftime(buf, 21, "%Y-%m-%dT%H:%M:%S.", std::localtime(&now_c));
  sprintf(buf+20, "%09ld", point.time_since_epoch().count() % 1000000000);
  return out;
}

void output::StorageService::stop() {
  if (_isStoring) {
    std::lock_guard<std::mutex> lockGuard(_storageServiceMtx);
    _isStoring = false;
    _outputStorage.setStoreCtrlData(false);
  }
}

void output::StorageService::start() {
  if (!_isStoring) {
    std::lock_guard<std::mutex> lockGuard(_storageServiceMtx);
    // Create folder to store rec into
    std::ostringstream folderName;
    _currFilePath = _storageBasePath + "/rec_" + formatTimePoint(std::chrono::system_clock::now()) + ".capnp.bin";
    _lastSavedTs = -1;
    _isStoring = true;
    _outputStorage.setStoreCtrlData(true);
    
    std::thread dataStorageThread(&output::StorageService::run, this);
    dataStorageThread.detach();
  }
}

void output::StorageService::run() {
  while (_isStoring) {
    {
      std::lock_guard<std::mutex> lockGuard(_storageServiceMtx);
      int64_t currAlgoTs = _outputStorage.getAlgoTs();
      if (currAlgoTs > _lastSavedTs) {
        _lastSavedTs = currAlgoTs;
        saveFrame();
      }
    }
    // Polling every 5 ms to check if there is new data
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
}

void output::StorageService::saveFrame() {
  if (_isStoring) {
    int fd = open(_currFilePath.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0755);
    if (_outputStorage.writeToFile(fd)) {
      close(fd);
    }
  }
}
