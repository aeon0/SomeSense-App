#include "rec_cam.h"
#include <iostream>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <thread>


data_reader::RecCam::RecCam(
  const std::string name,
  const double horizontalFov,
  const int width,
  const int height
) : 
  BaseCam(name, std::chrono::high_resolution_clock::now()),
  _gotOneFrame(false),
  _pause(true),
  _jumpToFrame(false)
{
  setCamIntrinsics(width, height, horizontalFov);
}

void data_reader::RecCam::handleRequest(const std::string& requestType, const nlohmann::json& requestData, nlohmann::json& responseData) {
  if (requestData["type"] == "client.get_ctrl_data") {
    // Tell client info about the current recording
    responseData["rec_info"] = {
      {"is_rec", true},
      {"rec_length", _recLength},
      {"is_playing", !_pause},
    };
  }
  else if (requestData["type"] == "client.play_rec") {
    std::lock_guard<std::mutex> lockGuardCtrls(_controlsMtx);
    _pause = false;
    responseData["success"] = true;
  }
  else if (requestData["type"] == "client.pause_rec") {
    std::lock_guard<std::mutex> lockGuardCtrls(_controlsMtx);
    _pause = true;
    responseData["success"] = true;
  }
  else if (requestData["type"] == "client.step_forward" && _pause) {
    std::lock_guard<std::mutex> lockGuardCtrls(_controlsMtx);
    _stepForward = true;
    responseData["success"] = true;
  }
  else if (requestData["type"] == "client.step_backward" && _pause) {
    std::lock_guard<std::mutex> lockGuardCtrls(_controlsMtx);
    _newFrameNr = _currFrameNr - 1;
    _jumpToFrame = true;
    responseData["success"] = true;
  }
  else if (requestData["type"] == "client.jump_to_ts") {
    std::lock_guard<std::mutex> lockGuardCtrls(_controlsMtx);
    const int64_t newTs = requestData["data"];
    // _newFrameNr = -1;
    // TODO: Find new frameNr from timestamps
    _jumpToFrame = true;
    _pause = true; // Also pause recording in case it was playing
    responseData["success"] = true;
  }
}

void data_reader::RecCam::start() {
  // Start thread to read image and store it into _currFrame
  std::thread dataReaderThread(&data_reader::RecCam::readData, this);
  dataReaderThread.detach();
}

void data_reader::RecCam::readData() {
  _currFrameNr = 0;
  int currFramePos = 0;
  int64_t startSensorTs = -1;
  int64_t lastSensorTs = -1;
  auto timeLastFrameRead = std::chrono::high_resolution_clock::now();

  for (;;) {
    if (_currFrameNr < _frames.size() && (!_pause || _stepForward || !_gotOneFrame || _jumpToFrame)) {
      auto frame = _frames[_currFrameNr];
      // Frame syncing
      if (startSensorTs == -1) {
        startSensorTs = frame->getTimestamp();
      }
      else {
        // Wait at least the time till current timestamp to sync sensor times
        std::chrono::microseconds diffSensorTs(frame->getTimestamp() - lastSensorTs);
        const auto sensorFrameEndTime = timeLastFrameRead + diffSensorTs;
        std::this_thread::sleep_until(sensorFrameEndTime);
      }
      timeLastFrameRead = std::chrono::high_resolution_clock::now();
      lastSensorTs = frame->getTimestamp();

      // Read and set img data
      const int imgWidth = frame->getImg().getWidth();
      const int imgHeight = frame->getImg().getHeight();
      // const int channels = frame->getImg().getChannels();
      const auto rawImgData = frame->getImg().getData();
      auto bufferMat = cv::Mat(cv::Size(imgWidth, imgHeight), CV_8UC3);
      memcpy(bufferMat.data, rawImgData.begin(), rawImgData.size());
      {
        std::lock_guard<std::mutex> lockGuardRead(_readMutex);
        _currFrame = bufferMat.clone();
        _validFrame = true;
        _currFrameNr++;
        _currTs = frame->getTimestamp() - startSensorTs;
      }
      bufferMat.release();
      _gotOneFrame = true;
      {
        std::lock_guard<std::mutex> lockGuardCtrls(_controlsMtx);
        _stepForward = false;
        _jumpToFrame = false;
      }
    }
  }

  // for (;;) {
    // if (!_pause || _stepForward || !_gotOneFrame || _jumpToFrame) {
    //   if (_jumpToFrame && _newFrameNr >= 0) {
    //     // Algo also has a requestHandler and resets Algo on jump_to_ts and step_backward
    //     // std::lock_guard<std::mutex> lockGuardCtrls(_controlsMtx);
    //     // _stream.set(cv::CAP_PROP_POS_FRAMES, _newFrameNr);
    //   }

      // const int currFrameNr = _stream.get(cv::CAP_PROP_POS_FRAMES);
      // const bool success = _stream.read(_bufferFrame);
      // if (success) {
      //   std::lock_guard<std::mutex> lockGuardRead(_readMutex);
      //   _currFrame = _bufferFrame.clone();
      //   _bufferFrame.release();
      //   _validFrame = success;

      //   std::lock_guard<std::mutex> lockGuardCtrls(_controlsMtx);
      //   if (_timestamps.size() == 0) {
      //     const double tsMsec = _stream.get(cv::CAP_PROP_POS_MSEC);
      //     _currTs = static_cast<int64>(tsMsec * 1000.0);
      //     _currFrameNr = currFrameNr;
      //   }
      //   else {
      //     _currTs = _timestamps.at(currFrameNr);
      //     _currFrameNr = currFrameNr;
      //   }
      // }

      // Wait the amount of time to the next timestamp or one frame length if timestamps are not available
      // TODO: Wait this time from the beginning of the frame including the fetching of the frame and other meta data
      // TODO: There is still something wrong when approching the end of the recording and going beyond, it does not stop
      // int64_t waitTimeUs = 0;
      // if (_timestamps.size() > (currFrameNr + 1)) {
      //   waitTimeUs = _timestamps.at(currFrameNr + 1) - _timestamps.at(currFrameNr);
      // }
      // else {
      //   waitTimeUs = (1 / _frameRate) * 1000000;
      // }
      // std::this_thread::sleep_for(std::chrono::microseconds(5000));

      // _gotOneFrame = true;
      // {
      //   std::lock_guard<std::mutex> lockGuardCtrls(_controlsMtx);
      //   _stepForward = false;
      //   _jumpToFrame = false;

      //   if (_currTs >= _recLength) {
      //     _pause = true;
      //     _outputStorage.setRecCtrlData(true, !_pause, _recLength);
      //   }
      // }
    // }
  // }
}
