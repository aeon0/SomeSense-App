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
    _frameRate = (_recLength / 1000000) / _timestamps.size();
  }

  output::CtrlData ctrlData;
  ctrlData.isStoring = false;
  ctrlData.isARecording = true;
  ctrlData.isPlaying = _pause;
  ctrlData.recLength = _recLength;
  _outputStorage.set(ctrlData);

  // Start thread to read image and store it into _currFrame
  std::thread dataReaderThread(&data_reader::VideoCam::readData, this);
  dataReaderThread.detach();
}

void data_reader::VideoCam::handleRequest(const std::string& requestType, const nlohmann::json& requestData, nlohmann::json& responseData) {
  if (requestData["type"] == "client.start_recording") {
    std::cout << "Start Recording" << std::endl;
    _pause = false;
    output::CtrlData ctrlData = _outputStorage.getCtrlData();
    ctrlData.isPlaying = _pause;
    _outputStorage.set(ctrlData);
  }
  else if (requestData["type"] == "client.stop_recording") {
    std::cout << "Stop Recording" << std::endl;
    _pause = true;
    output::CtrlData ctrlData = _outputStorage.getCtrlData();
    ctrlData.isPlaying = _pause;
    _outputStorage.set(ctrlData);
  }
}

void data_reader::VideoCam::readData() {
  for (;;) {
    if (!_pause || !_gotOneFrame) {
      _gotOneFrame = true;

      // Read frame and wait the amount of time it takes to get to the next timestamp
      const int currFrameNr = _stream.get(cv::CAP_PROP_POS_FRAMES);
      bool success = _stream.read(_bufferFrame);

      // Brackets are needed to release the lock
      {
        std::lock_guard<std::mutex> lockGuard(_readMutex);
        _currFrame = _bufferFrame.clone();
        _bufferFrame.release();
        _validFrame = success;

        if (_timestamps.size() == 0) {
          const double tsMsec = _stream.get(cv::CAP_PROP_POS_MSEC);
          _currTs = static_cast<int64>(tsMsec * 1000.0);
        }
        else {
          _currTs = _timestamps.at(currFrameNr);
        }
      }

      // Wait the amount of time to the next timestamp or one frame length if timestamps are not available
      int64_t waitTimeUs = 0;
      if (_timestamps.size() > (currFrameNr + 1)) {
        waitTimeUs = _timestamps.at(currFrameNr + 1) - _timestamps.at(currFrameNr);
      }
      else {
        waitTimeUs = (1 / _frameRate) * 1000000;
      }
      std::this_thread::sleep_for(std::chrono::microseconds(waitTimeUs));
    }
  }

  // if (updateToAlgoTs) {
  //   double newTs = static_cast<double>(currentAlgoTs) / 1000; // in [ms]
  //   const double lastPossibleTs = _recLength / 1000.0;
  //   newTs = std::clamp<double>(newTs, 0.0, lastPossibleTs);
  //   _stream.set(cv::CAP_PROP_POS_MSEC, newTs);
  // }
}
