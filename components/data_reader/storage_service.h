#pragma once

#include "opencv2/opencv.hpp"
#include <string>
#include <iostream>
#include <map>
#include <mutex>
#include "sensor_storage.h"
#include "utilities/json.hpp"


namespace data_reader {

  // Storing mp4 video files (for ICam objects) alongside index parallel timestamp information.
  // The timestamp information is in a .json file were each sensor (key == _sensorSotrage key)
  // has a index parallel timestamp array. E.g. frame 5 in the mp4 of the "FrontCam0" has its
  // exact timestamp in _timestamps["FrontCam0"][5].
  class StorageService {
  public:
    StorageService(const std::string storageBasePath, const SensorStorage& sensorStorage);

    void startStoring();
    void saveFrame();
    void stopStoring();

    bool isStoring() const;

  private:
    // Format a chrono timepoint in something such as "2019-12-27T21:11:13.134689566"
    static std::string formatTimePoint(std::chrono::system_clock::time_point point);

    bool _isStoring;
    int64 _startTs; // reference TS of the beginning of the recording
    std::string _currentStoragePath;
    const std::string _storageBasePath;
    const SensorStorage& _sensorStorage;
    std::map<const std::string, cv::VideoWriter> _videoWriters; // keys corresponds to _sensorStorage.getCams() keys
    nlohmann::json _timestamps;
  };
}
