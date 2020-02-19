#include "storage_service.h"
#include <chrono>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <fstream>
#include <atomic>


// Atomic bools as user interaction can start and stop storing at any time
std::atomic<bool> isStoring(false);
std::mutex storageServiceLock;


output::StorageService::StorageService(const std::string storageBasePath, const output::Storage& outputStorage) :
  _storageBasePath(storageBasePath), _outputStorage(outputStorage), _startTs(-1), _lastSavedTs(-1) {}

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
  if (isStoring) {
    std::lock_guard<std::mutex> lockGuard(storageServiceLock);

    for (auto& [key, writer]: _videoWriters) {
      writer.release();
    }
    _videoWriters.clear();

    // Save timestamps to file
    std::ofstream timestampFile(_currentStoragePath + "/timestamps.json");
    timestampFile << _timestamps.dump();
    timestampFile.close();
    _timestamps.clear();

    isStoring = false;
  }
}

void output::StorageService::start() {
  // In case we are already in "storing mode" do nothing
  if (!isStoring) {
    isStoring = true;

    std::ostringstream folderName;
    _currentStoragePath = _storageBasePath + formatTimePoint(std::chrono::system_clock::now());
    mkdir(_currentStoragePath.c_str(), 0755);

    _lastSavedTs = -1;
    _startTs = -1;

    std::thread dataStorageThread(&output::StorageService::run, this);
    dataStorageThread.detach();
  }
}

void output::StorageService::run() {
  while (isStoring) {
    int64_t currAlgoTs = _outputStorage.getAlgoTs();
    if (currAlgoTs > _lastSavedTs) {
      _lastSavedTs = currAlgoTs;
      saveFrame();
    }

    // Polling every 5 ms to check if there is new data
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
}

void output::StorageService::saveFrame() {
  if (isStoring) {
    std::lock_guard<std::mutex> lockGuard(storageServiceLock);

    // Get all images from the output storage
    Storage::CamImgMap camImgMap;
    _outputStorage.getCamImgs(camImgMap);

    // Check if videoWriter and timestamp array already exist and create in case it does not
    for (auto& [key, cam]: camImgMap) {
      if (_videoWriters.count(key) <= 0) {
        // TODO: add fps to camera? Not really needed in algo as its synced with timestamp, but just to look at mp4?
        // const double fps = cam.framerate;
        const cv::Size frameSize(cam.width, cam.height);
        const std::string videoFilePath = _currentStoragePath + "/" + key + ".mp4";

        auto writer = cv::VideoWriter(videoFilePath, cv::VideoWriter::fourcc('m','p','4','v'), 30, frameSize);
        _videoWriters.insert({key, writer});

        // For each videoWriter create a key array in the json object
        _timestamps[key] = nlohmann::json::array();
      }
    }

    // If the _startTs is not yet set (== -1), we need to find the smallest ts of the current frame
    // and set it as a reference to calculate a relative ts for all comming frames
    if (_startTs == -1) {
      for (auto& [key, writer]: _videoWriters) {
        const int64_t ts = camImgMap.at(key).timestamp;
        if (_startTs == -1 || ts < _startTs) {
          _startTs = ts;
        }
      }
    }

    // Store frame for all the cameras and store a relative timestamp to the json object
    for (auto& [key, writer]: _videoWriters) {
      writer.write(camImgMap.at(key).img);
      const int64_t relativeTs = camImgMap.at(key).timestamp - _startTs;
      _timestamps[key].push_back(relativeTs);
    }
  }
}
