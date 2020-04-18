#include "rec_cam.h"
#include <iostream>
#include <mutex>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <thread>


data_reader::RecCam::RecCam(
  const std::string name,
  const double horizontalFov,
  const int width,
  const int height,
  serialize::AppState& appState
) : 
  BaseCam(name, std::chrono::high_resolution_clock::now()),
  _gotOneFrame(false),
  _pause(true),
  _jumpToFrame(false),
  _appState(appState)
{
  setCamIntrinsics(width, height, horizontalFov);
}

void data_reader::RecCam::handleRequest(const std::string& requestType, const nlohmann::json& requestData, nlohmann::json& responseData) {
  // TODO: Save strings to constants to use across the code
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
      _currFrameNr -= 2; // -2 because after each frame that is read, _currFrameNr is already counted up for the next frame
      _jumpToFrame = true;
      _pause = true;
    }
  }
  else if (requestData["type"] == "client.jump_to_ts") {
    const int64_t newTs = requestData["data"];
    // Let's assume a more or less even frame rate, with that we should get the frameNr close to the desired one
    int frameNr = (static_cast<double>(newTs) / static_cast<double>(_recLength)) * _frames.size();
    bool found = false;
    while (!found) {
      int64_t startTs = _frames[0]->getTimestamp();
      int64_t tsDiff = std::abs(_frames[frameNr]->getTimestamp() - startTs - newTs);
      int64_t tsDiffNext = tsDiff + 1;
      int64_t tsDiffPrevious = tsDiff + 1;
      if (frameNr + 1 < _frames.size()) {
        tsDiffNext = std::abs(_frames[frameNr+1]->getTimestamp()- startTs - newTs);
      }
      if (frameNr - 1 >= 0) {
        tsDiffPrevious = std::abs(_frames[frameNr-1]->getTimestamp() - startTs - newTs);
      }
      if (tsDiff < tsDiffNext && tsDiff < tsDiffPrevious) {
        found = true;
      }
      else if (tsDiffNext < tsDiffPrevious){
        frameNr++;
      }
      else {
        frameNr--;
      }
    }
    _currFrameNr = frameNr;
    _jumpToFrame = true;
    _pause = true; // Also pause recording in case it was playing
  }
  // Set rec info to app state to inform client about changes
  // Might have performance issues if clients make a lot of requests
  _appState.setRecState(true, _recLength, !_pause);
}

void data_reader::RecCam::start() {
  _appState.setRecState(true, _recLength, !_pause);

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
      else if (!_jumpToFrame && !_stepForward) {
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
      _jumpToFrame = false;
      _stepForward = false;

      if (_currFrameNr >= _frames.size()) {
        _pause = true;
        _appState.setRecState(true, _recLength, !_pause);
      }
    }
  }
}
