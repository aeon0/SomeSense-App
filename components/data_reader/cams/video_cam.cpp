#include "video_cam.h"
#include <fstream>
#include <string>
#include <unordered_map>
#include <iostream>
#include <mutex>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <thread>
#include <capnp/serialize.h>
#include "utilities/json.hpp"


data_reader::VideoCam::VideoCam(
  const std::string name,
  const std::string& filePath,
  serialize::AppState& appState
) :
  BaseCam(name, std::chrono::high_resolution_clock::now()),
  _appState(appState),
  _gotOneFrame(false),
  _pause(true),
  _jumpToFrame(false),
  _currFrameNr(0),
  _filePath(filePath)
{
  _stream.open(_filePath, cv::CAP_FFMPEG);
  if (!_stream.isOpened()) {
    throw std::runtime_error("VideoCam could not open file: " + _filePath);
  }

  _frameSize = cv::Size(_stream.get(cv::CAP_PROP_FRAME_WIDTH), _stream.get(cv::CAP_PROP_FRAME_HEIGHT));
  _frameRate = _stream.get(cv::CAP_PROP_FPS);
  const double frameCount = _stream.get(cv::CAP_PROP_FRAME_COUNT);
  _recLength = static_cast<int64>(((frameCount - 1) / _frameRate) * 1000000);

  setIntrinsics(_stream.get(cv::CAP_PROP_FRAME_WIDTH), _stream.get(cv::CAP_PROP_FRAME_HEIGHT), 1.5);
}

void data_reader::VideoCam::handleRequest(const std::string& requestType, const nlohmann::json& requestData, nlohmann::json& responseData) {
  if (
    requestData["type"] == "client.play_rec" ||
    requestData["type"] == "client.pause_rec" ||
    requestData["type"] == "client.step_forward" ||
    requestData["type"] == "client.step_backward" ||
    requestData["type"] == "client.jump_to_ts"
  ) {
    std::unique_lock<std::mutex> lockGuard(_readLock);

    if (requestData["type"] == "client.play_rec") {
      _pause = false;
    }
    else if (requestData["type"] == "client.pause_rec") {
      _pause = true;
    }
    else if (requestData["type"] == "client.step_forward") {
      _stepForward = true;
      _pause = true;
    }
    else if (requestData["type"] == "client.step_backward") {
      if (_currFrameNr > 1) {
        _currFrameNr -= 2;
        _jumpToFrame = true;
        _pause = true;
      }
    }
    else if (requestData["type"] == "client.jump_to_ts") {
      const int64_t newTs = requestData["data"];
      _currFrameNr = static_cast<int>((static_cast<double>(newTs) / 1000000.0) * _frameRate);
      _jumpToFrame = true;
      _pause = true; // Also pause recording in case it was playing
    }

    responseData["isPlaying"] = !_pause;
    _appState.setRecData(!_pause, _recLength, true, false);
  }
}

void data_reader::VideoCam::start() {
  _appState.setRecData(!_pause, _recLength, true);

  // Start thread to read image and store it into _currFrame
  std::thread dataReaderThread(&data_reader::VideoCam::readData, this);
  dataReaderThread.detach();
}

void data_reader::VideoCam::readData() {
  for (;;) {
    if (!_pause || _stepForward || !_gotOneFrame || _jumpToFrame) {
      if (_jumpToFrame && _currFrameNr >= 0) {
        _stream.set(cv::CAP_PROP_POS_FRAMES, _currFrameNr);
      }

      const int currFrameNr = _stream.get(cv::CAP_PROP_POS_FRAMES);
      const bool success = _stream.read(_bufferFrame);
      if (success) {
        std::lock_guard<std::mutex> lockGuardRead(_readMutex);
        _currFrame = _bufferFrame.clone();
        _bufferFrame.release();
        _validFrame = success;

        const double tsMsec = _stream.get(cv::CAP_PROP_POS_MSEC);
        _currTs = static_cast<int64>(tsMsec * 1000.0);
        _currFrameNr = currFrameNr;
      }

      // Wait the amount of time to the next timestamp or one frame length if timestamps are not available
      // TODO: Wait this time from the beginning of the frame including the fetching of the frame and other meta data
      // TODO: There is still something wrong when approching the end of the recording and going beyond, it does not stop
      int64_t waitTimeUs = 0;
      if (_timestamps.size() > (currFrameNr + 1)) {
        waitTimeUs = _timestamps.at(currFrameNr + 1) - _timestamps.at(currFrameNr);
      }
      else {
        waitTimeUs = (1 / _frameRate) * 1000000;
      }
      std::this_thread::sleep_for(std::chrono::microseconds(waitTimeUs));

      _gotOneFrame = true;
      _stepForward = false;
      _jumpToFrame = false;

      if (_currTs >= _recLength) {
        _pause = true;
        _appState.setRecData(!_pause, _recLength, true);
      }
    }
  }
}
