#include "storage_service.h"
#include <chrono>
#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>

// Mutex needed because user interactions can fire startStoring(), stopStoring() at any time from the server thread
std::mutex storageServiceLock;


data_reader::StorageService::StorageService(const std::string storageBasePath, const SensorStorage& sensorStorage) :
  _isStoring(false), _storageBasePath(storageBasePath), _sensorStorage(sensorStorage), _startTs(-1) {}

std::string data_reader::StorageService::formatTimePoint(std::chrono::system_clock::time_point point) {
    static_assert(std::chrono::system_clock::time_point::period::den == 1000000000 && std::chrono::system_clock::time_point::period::num == 1);
    std::string out(29, '0');
    char* buf = &out[0];
    std::time_t now_c = std::chrono::system_clock::to_time_t(point);
    std::strftime(buf, 21, "%Y-%m-%dT%H:%M:%S.", std::localtime(&now_c));
    sprintf(buf+20, "%09ld", point.time_since_epoch().count() % 1000000000);
    return out;
}

bool data_reader::StorageService::isStoring() const {
  // Normaly this should not be needed for returning a bool, but better save than sorry
  std::lock_guard<std::mutex> lockGuard(storageServiceLock);
  return _isStoring;
}

void data_reader::StorageService::startStoring() {
  if (!_isStoring) {
    std::lock_guard<std::mutex> lockGuard(storageServiceLock);
    _isStoring = true;

    // Create a new folder for storing the data based on the current date + time
    std::ostringstream folderName;
    _currentStoragePath = _storageBasePath + formatTimePoint(std::chrono::system_clock::now());
    mkdir(_currentStoragePath.c_str(), 0755);

    // Loop through all sensors in the storage and create e.g. cv::VideoWriter for each camera
    for (auto const& [key, cam]: _sensorStorage.getCams()) {
      const double fps = cam->getFrameRate();
      const cv::Size frameSize = cam->getFrameSize();
      const std::string videoFilePath = _currentStoragePath + "/" + key + ".mp4";

      auto writer = cv::VideoWriter(videoFilePath, cv::VideoWriter::fourcc('m','p','4','v'), fps, frameSize);
      _videoWriters.insert({key, writer});

      // For each videoWriter create a key array in the json object
      _timestamps[key] = nlohmann::json::array();
    }

    // Set to -1 to force a reset on first time calling saveFrame()
    _startTs = -1;
  }
}

void data_reader::StorageService::stopStoring() {
  if (_isStoring) {
    std::lock_guard<std::mutex> lockGuard(storageServiceLock);
    _isStoring = false;

    for (auto& [key, writer]: _videoWriters) {
      writer.release();
    }
    _videoWriters.clear();

    // Save timestamps to file
    std::ofstream timestampFile(_currentStoragePath + "/timestamps.json");
    timestampFile << _timestamps.dump();
    timestampFile.close();
    _timestamps.clear();
  }
}

void data_reader::StorageService::saveFrame() {
  if (_isStoring) {
    std::lock_guard<std::mutex> lockGuard(storageServiceLock);

    // If the _startTs is not yet set (== -1), we need to find the smallest ts of the current frame
    // and set it as a reference to calculate a relative ts for all comming frames
    if (_startTs == -1) {
      for (auto& [key, writer]: _videoWriters) {
        auto [success, ts, frame] = _sensorStorage.getCams().at(key)->getFrame();
        if (success && (_startTs == -1 || ts < _startTs)) {
          _startTs = ts;
        }
      }
    }

    // Store frame for all the cameras and store a relative timestamp to the json object
    for (auto& [key, writer]: _videoWriters) {
      auto [success, ts, frame] = _sensorStorage.getCams().at(key)->getFrame();
      if (success) {
        writer.write(frame);
        const int64 relativeTs = ts - _startTs;
        _timestamps[key].push_back(relativeTs);
      }
    }
  }
}
