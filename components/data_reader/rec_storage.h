#pragma once

#include "opencv2/opencv.hpp"
#include <string>
#include <iostream>
#include <map>
#include "sensor_storage.h"


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
    std::string _currentStoragePath;
    const std::string _storageBasePath;
    const SensorStorage& _sensorStorage;
    std::map<const std::string, cv::VideoWriter> _videoWriters; // key corresponds to _sensorStorage.getCams()
  };
}
