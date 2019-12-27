#pragma once

#include "opencv2/opencv.hpp"
#include <string>
#include <iostream>
#include <map>
#include <mutex>
#include "sensor_storage.h"
#include "utilities/json.hpp"


namespace data_reader {
  class RecStorage {
  public:
    RecStorage(const std::string storageBasePath, const SensorStorage& sensorStorage);

    void startStoring();
    void saveFrame();
    void stopStoring();

    bool isStoring() const { return _isStoring; }

  private:
    static std::string formatTimePoint(std::chrono::system_clock::time_point point);

    bool _isStoring;
    int64 _startTs; // reference TS to start
    std::string _currentStoragePath;
    const std::string _storageBasePath;
    const SensorStorage& _sensorStorage;
    std::map<const std::string, cv::VideoWriter> _videoWriters; // key corresponds to _sensorStorage.getCams()
    nlohmann::json _timestamps;
  };
}
