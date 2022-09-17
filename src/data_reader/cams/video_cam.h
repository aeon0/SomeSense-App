#pragma once

#include <atomic>
#include <tuple>
#include "base_cam.h"
#include "opencv2/opencv.hpp"
#include "serialize/app_state.h"
#include "com_out/irequest_listener.h"


namespace data_reader {
  class VideoCam : public BaseCam, public com_out::IRequestListener {
  public:
    VideoCam(const std::string name, const std::string& filePath, serialize::AppState& appState);

    void start() override;
    void handleRequest(const std::string& requestType, const nlohmann::json& requestData, nlohmann::json& responseData) override;
    bool isRecording() const override { return true; }
    int64_t getRecLength() const override { return _recLength; }

  private:
    void readData();

    cv::VideoCapture _stream;
    int64_t _recLength; // length of recording in [us]

    bool _gotOneFrame; // At least one frame needs to be created in the beginning
    bool _pause; // Pause the video
    bool _stepForward; // One step forward
    bool _jumpToFrame; // Jump to the frame number
    int _currFrameNr; // Count of frames and used to tell were to jump to for (jump_to_ts and step_backward)

    serialize::AppState& _appState;
    std::string _filePath;
    std::vector<off_t> _msgStarts;
    std::vector<int64_t> _timestamps;

    std::mutex _readLock;
    std::atomic<bool> _finishedFrame;
  };
}
