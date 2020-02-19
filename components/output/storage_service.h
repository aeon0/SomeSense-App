#pragma once

#include "opencv2/opencv.hpp"
#include <string>
#include <iostream>
#include <map>
#include <mutex>
#include "utilities/json.hpp"
#include "output/storage.h"
#include "com_out/irequest_listener.h"


namespace output {

  // Storing mp4 video files (for ICam objects) alongside index parallel timestamp information.
  // The timestamp information is in a .json file were each sensor (key == _sensorSotrage key)
  // has a index parallel timestamp array. E.g. frame 5 in the mp4 of the "FrontCam0" has its
  // exact timestamp in _timestamps["FrontCam0"][5].
  class StorageService : public com_out::IRequestListener {
  public:
    StorageService(const std::string storageBasePath, const output::Storage& outputStorage);

    void handleRequest(const std::string& requestType, const nlohmann::json& requestData, nlohmann::json& responseData) override;
    void saveFrame();
    void run();
    void stop();

  private:
    // Format a chrono timepoint in something such as "2019-12-27T21:11:13.134689566" for unique file name creation
    static std::string formatTimePoint(std::chrono::system_clock::time_point point);

    const output::Storage& _outputStorage;
    int64_t _startTs; // reference TS of the beginning of the recording
    std::string _currentStoragePath;
    const std::string _storageBasePath;
    std::map<const std::string, cv::VideoWriter> _videoWriters; // keys corresponds to _sensorStorage.getCams() keys
    nlohmann::json _timestamps;
    int64_t _lastSavedTs;
  };
}
