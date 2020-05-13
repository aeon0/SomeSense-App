#pragma once

#include <tuple>
#include <map>
#include <atomic>
#include "base_cam.h"
#include "com_out/irequest_listener.h"
#include "serialize/app_state.h"
#include "serialize/frame.capnp.h"


namespace data_reader {

  class RecCam : public BaseCam, public com_out::IRequestListener {
  public:
    RecCam(
      const std::string name,
      const double horizontalFov,
      const int width,
      const int height,
      serialize::AppState& appState,
      std::string filePath
    );

    void handleRequest(const std::string& requestType, const nlohmann::json& requestData, nlohmann::json& responseData) override;

    void start() override;

    // Recording specific getters (have default implementation otherwise)
    const bool isRecording() const override { return true; }
    const int64_t getRecLength() const override { return _recLength; }

  private:
    int findFrameNrFromTs(const int64_t ts);
    void readData();

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
