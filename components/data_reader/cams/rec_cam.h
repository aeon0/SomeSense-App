#pragma once

#include <tuple>
#include <map>
#include <atomic>
#include "base_cam.h"
#include "com_out/irequest_listener.h"
#include "serialize/app_state.h"
#include "serialize/frame.capnp.h"
#include "../rec/own_capnp.h"


namespace data_reader {

  class RecCam : public BaseCam, public com_out::IRequestListener {
  public:
    typedef data_reader::rec::OwnCapnp<CapnpOutput::CamSensor> OwnCamSensor;
    typedef std::vector<std::shared_ptr<OwnCamSensor>> OwnCamFrames;

    RecCam(
      const std::string name,
      const double horizontalFov,
      const int width,
      const int height,
      serialize::AppState& appState
    );

    void handleRequest(const std::string& requestType, const nlohmann::json& requestData, nlohmann::json& responseData) override;

    void start() override;

    void setFrameData(OwnCamFrames& frames) {
      _frames = std::move(frames);
      if (_frames.size() > 0) {
        _recLength = _frames[_frames.size() - 1]->getTimestamp() - _frames[0]->getTimestamp();
        _frameRate = (static_cast<double>(_frames.size()) / static_cast<double>(_recLength)) * 1000000.0;
      }
    };

    // Recording specific getters (have default implementation otherwise)
    const bool isRecording() const override { return true; }
    const int64_t getRecLength() const override { return _recLength; }

  private:
    void readData();

    int64_t _recLength; // length of recording in [us]

    bool _gotOneFrame; // At least one frame needs to be created in the beginning
    std::atomic<bool> _pause; // Pause the video
    std::atomic<bool> _stepForward; // One step forward
    std::atomic<bool> _jumpToFrame; // Jump to the frame number

    OwnCamFrames _frames;
    serialize::AppState& _appState;
  };
}
