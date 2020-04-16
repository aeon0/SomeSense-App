#pragma once

#include <mutex>
#include <tuple>
#include <map>
#include "output/storage.h"
#include "base_cam.h"
#include "com_out/irequest_listener.h"
#include "output/frame.capnp.h"


namespace data_reader {
  template <typename T>
  class OwnCapnp: public T::Reader {
    // A copy of a capnp object which lives in-memory and can be passed by ownership.

  public:
    // Inherits methods of reader.

  private:
    kj::Array<capnp::word> words;

    OwnCapnp(kj::Array<capnp::word> words)
        : T::Reader(capnp::readMessageUnchecked<T>(words.begin())),
          words(kj::mv(words)) {}

    template <typename Reader>
    friend OwnCapnp<capnp::FromReader<Reader>> newOwnCapnp(Reader value);
  };

  template <typename Reader>
  OwnCapnp<capnp::FromReader<Reader>> newOwnCapnp(Reader value) {
    auto words = kj::heapArray<capnp::word>(value.totalSize().wordCount + 1);
    memset(words.asBytes().begin(), 0, words.asBytes().size());
    capnp::copyToUnchecked(value, words);
    return OwnCapnp<capnp::FromReader<Reader>>(kj::mv(words));
  }

  class RecCam : public BaseCam, public com_out::IRequestListener {
  public:
    RecCam(
      const std::string name,
      const double horizontalFov,
      const int width,
      const int height,
      const std::string recFilePath
    );

    void handleRequest(const std::string& requestType, const nlohmann::json& requestData, nlohmann::json& responseData) override;

    void start() override;

    // Recording specific getters (have default implementation otherwise)
    const bool isRecording() const override { return true; }
    const int64_t getRecLength() const override { return _recLength; }

  private:
    void readData();

    const std::string _recFilePath;

    int64_t _recLength; // length of recording in [us]

    std::mutex _controlsMtx;
    bool _gotOneFrame; // At least one frame needs to be created in the beginning
    bool _pause; // Pause the video
    bool _stepForward; // One step forward
    bool _jumpToFrame; // Jump to the frame number that is in _newFrameNr
    int _newFrameNr;

    std::vector<std::shared_ptr<data_reader::OwnCapnp<CapnpOutput::Frame>>> _frames;
  };
}
