#include "video_cam.h"
#include <iostream>
#include <algorithm>
#include <thread>


data_reader::VideoCam::VideoCam(const std::string name, const TS& algoStartTime, output::Storage& outputStorage, const std::string& filename, const std::vector<int64> timestamps) :
    BaseCam(name, algoStartTime), _outputStorage(outputStorage), _filename(filename), _timestamps(timestamps), _gotOneFrame(false), _pause(true) {
  _stream.open(_filename);
  if (!_stream.isOpened()) {
    throw std::runtime_error("VideoCam could not open file: " + _filename);
  }

  _frameSize = cv::Size(_stream.get(cv::CAP_PROP_FRAME_WIDTH), _stream.get(cv::CAP_PROP_FRAME_HEIGHT));

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

  output::CtrlData ctrlData;
  ctrlData.isStoring = false;
  ctrlData.isARecording = true;
  ctrlData.isPlaying = !_pause;
  ctrlData.recLength = _recLength;
  _outputStorage.set(ctrlData);

  // Start thread to read image and store it into _currFrame
  std::thread dataReaderThread(&data_reader::VideoCam::readData, this);
  dataReaderThread.detach();
}

void data_reader::VideoCam::handleRequest(const std::string& requestType, const nlohmann::json& requestData, nlohmann::json& responseData) {
  // TODO: Pause has quite the delay, why?? Probably bad computer performancE?
  std::lock_guard<std::mutex> lockGuardCtrls(_controlsMtx);
  if (requestData["type"] == "client.play_rec") {
    _pause = false;
    output::CtrlData ctrlData = _outputStorage.getCtrlData();
    ctrlData.isPlaying = !_pause;
    _outputStorage.set(ctrlData);
  }
  else if (requestData["type"] == "client.pause_rec") {
    std::cout << "Pause Recording" << std::endl;
    _pause = true;
    output::CtrlData ctrlData = _outputStorage.getCtrlData();
    ctrlData.isPlaying = !_pause;
    _outputStorage.set(ctrlData);
  }
  else if (requestData["type"] == "client.step_forward" && _pause) {
    _stepForward = true;
  }
  else if (requestData["type"] == "client.step_backward" && _pause) {
    std::cout << "Step Backward" << std::endl;
    if (_timestamps.size() == 0) {
      const int64_t frameLengthUs = static_cast<int64_t>((1.0 / _frameRate) * 1000000);
      _newTs = _currTs - (2 * frameLengthUs);
      _jumpToTs = true;
    }
    else {
      const int currFrameNr = _stream.get(cv::CAP_PROP_POS_FRAMES);
      if (currFrameNr > 0) {
         _newTs = _timestamps.at(currFrameNr - 1) - 1;
         _jumpToTs = true;
      }
    }
  }
  else if (requestData["type"] == "client.jump_to_ts") {
    std::cout << "Jump to Ts" << std::endl;
    _newTs = requestData["data"];
    _jumpToTs = true;
  }
}

void data_reader::VideoCam::readData() {
  for (;;) {
    if (!_pause || _stepForward || !_gotOneFrame || _jumpToTs) {
      std::cout << "Read Frame" << std::endl;

      if (_jumpToTs) {
        // TODO: Algo needs to be reset in this case! Maybe check in algo if timestamp is in past. And if it is reset?
        std::lock_guard<std::mutex> lockGuardCtrls(_controlsMtx);
        const double newTsMs = _newTs / 1000.0;
        _stream.set(cv::CAP_PROP_POS_MSEC, static_cast<int>(newTsMs));
      }

      const int currFrameNr = _stream.get(cv::CAP_PROP_POS_FRAMES);
      const bool success = _stream.read(_bufferFrame);

      // Brackets are needed to release the lock
      {
        std::lock_guard<std::mutex> lockGuardRead(_readMutex);
        _currFrame = _bufferFrame.clone();
        _bufferFrame.release();
        _validFrame = success;

        std::lock_guard<std::mutex> lockGuardCtrls(_controlsMtx);
        if (_timestamps.size() == 0) {
          const double tsMsec = _stream.get(cv::CAP_PROP_POS_MSEC);
          _currTs = static_cast<int64>(tsMsec * 1000.0);
        }
        else {
          _currTs = _timestamps.at(currFrameNr);
        }
      }

      // Wait the amount of time to the next timestamp or one frame length if timestamps are not available
      // TODO: Wait this time from the beginning of the frame including the fetching of the frame and other meta data
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
        _jumpToTs = false;
      }
    }
  }
}
