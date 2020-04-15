#include "video_cam.h"
#include <iostream>
#include <algorithm>
#include <thread>
#include "output/frame.capnp.h"


data_reader::VideoCam::VideoCam(const std::string name, const TS& algoStartTime, output::Storage& outputStorage, const std::string& filename, const double horizontalFov, const std::vector<int64> timestamps) :
    BaseCam(name, algoStartTime), _outputStorage(outputStorage), _filename(filename), _timestamps(timestamps), _gotOneFrame(false), _pause(true), _jumpToFrame(false) {
  _stream.open(_filename);
  if (!_stream.isOpened()) {
    throw std::runtime_error("VideoCam could not open file: " + _filename);
  }

  setCamIntrinsics(_stream.get(cv::CAP_PROP_FRAME_WIDTH), _stream.get(cv::CAP_PROP_FRAME_HEIGHT), horizontalFov);

  // If timestamps are not filled, use camera fps info for timestamps, otherwise use the provided timestamps
  if (_timestamps.size() == 0) {
    _frameRate = _stream.get(cv::CAP_PROP_FPS);
    const double frameCount = _stream.get(cv::CAP_PROP_FRAME_COUNT);
    _recLength = static_cast<int64>(((frameCount - 1) / _frameRate) * 1000000);
  }
  else {
    _recLength = _timestamps.back() - _timestamps.front();
    _frameRate = static_cast<double>(_timestamps.size()) / (static_cast<double>(_recLength) / 1000000.0);
  }

  _outputStorage.setRecCtrlData(true, !_pause, _recLength);

  // Start thread to read image and store it into _currFrame
  std::thread dataReaderThread(&data_reader::VideoCam::readData, this);
  dataReaderThread.detach();
}

void data_reader::VideoCam::handleRequest(const std::string& requestType, const nlohmann::json& requestData, nlohmann::json& responseData) {
  std::lock_guard<std::mutex> lockGuardCtrls(_controlsMtx);
  if (requestData["type"] == "client.play_rec") {
    _pause = false;
    _outputStorage.setRecCtrlData(true, !_pause, _recLength);
  }
  else if (requestData["type"] == "client.pause_rec") {
    _pause = true;
    _outputStorage.setRecCtrlData(true, !_pause, _recLength);
  }
  else if (requestData["type"] == "client.step_forward" && _pause) {
    _stepForward = true;
  }
  else if (requestData["type"] == "client.step_backward" && _pause) {
    _newFrameNr = _currFrameNr - 1;
    _jumpToFrame = true;
  }
  else if (requestData["type"] == "client.jump_to_ts") {
    const int64_t newTs = requestData["data"];
    
    _newFrameNr = -1;

    if (_timestamps.size() == 0) {
      _newFrameNr = static_cast<int>((static_cast<double>(newTs) / 1000000.0) * _frameRate);
    }
    else {
      int64_t smallestDiff = INT64_MAX;
      // find frame that is closest to this ts, TODO: runtime optimize the search
      for (uint i = 0; i < _timestamps.size(); ++i)
      {
        int64_t diff = std::abs(_timestamps[i] - newTs);
        if (i == 0 || diff < smallestDiff) {
          smallestDiff = diff;
          _newFrameNr = i;
        }
      }
    }
    _jumpToFrame = true;
    _pause = true; // Also pause recording in case it was playing
    _outputStorage.setRecCtrlData(true, !_pause, _recLength);
  }
}

void data_reader::VideoCam::readData() {
  for (;;) {
    if (!_pause || _stepForward || !_gotOneFrame || _jumpToFrame) {
      if (_jumpToFrame && _newFrameNr >= 0) {
        // Algo also has a requestHandler and resets Algo on jump_to_ts and step_backward
        std::lock_guard<std::mutex> lockGuardCtrls(_controlsMtx);
        _stream.set(cv::CAP_PROP_POS_FRAMES, _newFrameNr);
      }

      const int currFrameNr = _stream.get(cv::CAP_PROP_POS_FRAMES);
      const bool success = _stream.read(_bufferFrame);
      if (success) {
        std::lock_guard<std::mutex> lockGuardRead(_readMutex);
        _currFrame = _bufferFrame.clone();
        _bufferFrame.release();
        _validFrame = success;

        std::lock_guard<std::mutex> lockGuardCtrls(_controlsMtx);
        if (_timestamps.size() == 0) {
          const double tsMsec = _stream.get(cv::CAP_PROP_POS_MSEC);
          _currTs = static_cast<int64>(tsMsec * 1000.0);
          _currFrameNr = currFrameNr;
        }
        else {
          _currTs = _timestamps.at(currFrameNr);
          _currFrameNr = currFrameNr;
        }
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
      {
        std::lock_guard<std::mutex> lockGuardCtrls(_controlsMtx);
        _stepForward = false;
        _jumpToFrame = false;

        if (_currTs >= _recLength) {
          _pause = true;
          _outputStorage.setRecCtrlData(true, !_pause, _recLength);
        }
      }
    }
  }
}
