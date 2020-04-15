#pragma once

#include <mutex>
#include <tuple>
#include "output/storage.h"
#include "base_cam.h"
#include "com_out/irequest_listener.h"


namespace data_reader {
  class RecCam : public BaseCam, public com_out::IRequestListener {
  public:
    RecCam(
      const std::string name,
      output::Storage& outputStorage,
      const double horizontalFov,
      const std::string recFilePath
    );

    void handleRequest(const std::string& requestType, const nlohmann::json& requestData, nlohmann::json& responseData) override;

    void start();

    // Recording specific getters (have default implementation otherwise)
    const bool isRecording() const override { return true; }
    const int64_t getRecLength() const override { return _recLength; }

  private:
    const std::string _recFilePath;

    output::Storage& _outputStorage;
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
