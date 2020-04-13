#pragma once

#include <mutex>
#include <tuple>
#include "output/storage.h"
#include "base_cam.h"
#include "com_out/irequest_listener.h"


namespace data_reader {
  class VideoCam : public BaseCam, public com_out::IRequestListener {
  public:
    VideoCam(const std::string name, const TS& algoStartTime, output::Storage& outputStorage, const std::string& filename, const double horizontalFov, const std::vector<int64> timestamps = {});

    void handleRequest(const std::string& requestType, const nlohmann::json& requestData, nlohmann::json& responseData) override;

    // Recording specific getters (have default implementation otherwise)
    const bool isRecording() const override { return true; }
    const int64_t getRecLength() const override { return _recLength; }

  private:
    output::Storage& _outputStorage;

    const std::string _filename;
    const std::vector<int64> _timestamps;

    cv::VideoCapture _stream;
    int64_t _recLength; // length of recording in [us]

    std::mutex _controlsMtx;
    bool _gotOneFrame; // At least one frame needs to be created in the beginning
    bool _pause; // Pause the video
    bool _stepForward; // One step forward
    bool _jumpToFrame; // Jump to the frame number that is in _newFrameNr
    int _newFrameNr;

    void readData();
  };
}
