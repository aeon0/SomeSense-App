#pragma once

#include "opencv2/opencv.hpp"
#include <string>
#include <mutex>
#include <atomic>
#include "utilities/json.hpp"
#include "serialize/app_state.h"
#include "com_out/irequest_listener.h"


namespace serialize {
  class SaveToFile : public com_out::IRequestListener {
  public:
    SaveToFile(const std::string storageBasePath, AppState& appState);

    void handleRequest(const std::string& requestType, const nlohmann::json& requestData, nlohmann::json& responseData) override;
    void saveFrame();

    void start();
    void stop();

  private:
    // start() calls this in a new thread
    void run();
    // Format a chrono timepoint in something such as "2019-12-27T21:11:13.134689566" for unique file name creation
    static std::string formatTimePoint(std::chrono::system_clock::time_point point);

    AppState& _appState;
    const std::string _storageBasePath;
    std::string _currFilePath;
    int64_t _lastSavedTs;
    std::atomic<bool> _isStoring;

    std::mutex _storageServiceMtx;
  };
}
