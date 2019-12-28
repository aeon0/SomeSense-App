#include "app.h"
#include "config.h"
#include <iostream>
#include <thread>
#include <cmath>
#include <chrono>
#include <algorithm>
#include "utilities/base64.h"
#include "utilities/json.hpp"

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

volatile sig_atomic_t stopFromSignal = 0;
void sighandler(int signum) { stopFromSignal = 1; }


frame::App::App(const std::string& sensorConfigPath) :
    _isRecording(false), _recLength(0), _pause(true), _stepBackward(false), _ts(0),
    _stepForward(false), _updateTs(false), _jumpToTs(-1), _outputState(""), _frame(-1),
    _storageService(Config::storagePath, _sensorStorage) {

  // Listen to SIGINT (usually ctrl + c on terminal) to stop endless algo loop
  signal(SIGINT, &sighandler);

  // _detector.loadModel("assets/od_model/model.onnx", "assets/od_model/prior_boxes.json");
  _sensorStorage.initFromConfig(sensorConfigPath);

  // Check if any of the sensors is playing from a recording
  for (auto const& [key, cam]: _sensorStorage.getCams()) {
    if (cam->isRecording()) {
      _isRecording = true;
      if (cam->getRecLength() > _recLength) {
        _recLength = cam->getRecLength();
        _maxFrames = static_cast<int>(_recLength / (Config::goalFrameLength * 1000.0));
      }
    }
  }
}

void frame::App::handleRequest(const std::string& requestType, const nlohmann::json& requestData, nlohmann::json& responseData) {
  if (requestType == "client.play_rec") {
    _pause = false;
    responseData["success"] = true;
  }
  else if (requestType == "client.pause_rec") {
    _pause = true;
    responseData["success"] = true;
  }
  else if (requestType == "client.step_forward") {
    _stepForward = true;
    responseData["success"] = true;
  }
  else if (requestType == "client.step_backward") {
    _stepBackward = true;
    responseData["success"] = true;
  }
  else if (requestType == "client.jump_to_ts") {
    _jumpToTs = static_cast<int64>(requestData["data"]);
    _updateTs = true;
    responseData["success"] = true;
  }
  else if (requestType == "client.start_storing") {
    _storageService.startStoring();
    responseData["success"] = true;
  }
  else if (requestType == "client.stop_storing") {
    _storageService.stopStoring();
    responseData["success"] = true;
  }
}

void frame::App::run(const com_out::IBroadcast& broadCaster) {
  const auto algoStartTime = std::chrono::high_resolution_clock::now();

  while (!stopFromSignal) {
    const auto frameStartTime = std::chrono::high_resolution_clock::now();

    // Check if a new frame should be created, note that pausing and stepping is only possible with recordings
    if (_outputState == "" || !_pause || !_isRecording || _stepForward || _stepBackward || _updateTs) {

      int64 videoAlgoTs = -1; // This is the "ideal" algo timestamp based on frame count and frame length
      if (_isRecording) {
        if (_stepBackward) {
          _frame--;
          // Backward is not the standard way to fetch video frames, thus use timestamp to get the frame
          _updateTs = true;
        }
        else if (_updateTs) {
          _frame = static_cast<int>(static_cast<double>(_jumpToTs) / (Config::goalFrameLength * 1000.0));
        }
        else {
          _frame++;
        }

        _frame = std::clamp<int>(_frame, 0, _maxFrames);
        if (_frame >= _maxFrames) {
          _pause = true; // pause in case the end of the recording is reached
          _updateTs = true;
        }
        // Calc the ideal timestamp based on frame number and frame length in [us]
        videoAlgoTs = static_cast<int64>(_frame * Config::goalFrameLength * 1000.0);
      }
      else {
        _frame++;
      }

      // Output State contains all data which is sent to the "outside" e.g. to visualize
      nlohmann::json jsonOutputState = {
        {"type", "server.frame"},
        {"data", {
          {"tracks", {}},
          {"sensors", {}},
          {"isRecording", _isRecording},
          {"recLength", _recLength},
          {"isPlaying", !_pause},
          {"isStoring", _storageService.isStoring()},
        }}
      };

      _ts = 0; // reset _ts, will be updated with the latest sensor ts

      for (auto const& [key, cam]: _sensorStorage.getCams()) {
        auto [success, sensorTs, img] = cam->getNewFrame(algoStartTime, videoAlgoTs, _updateTs);
        if (sensorTs > _ts) {
          _ts = sensorTs; // algo algo ts will be the latest sensorTs
        }

        if (success) {
          // Do processing per image
          // _detector.detect(img);

          // Some test data to send
          std::vector<uchar> buf;
          cv::imencode(".jpg", img, buf);
          auto *encMsg = reinterpret_cast<unsigned char*>(buf.data());
          std::string encodedBase64Img = base64_encode(encMsg, buf.size());
          encodedBase64Img = "data:image/jpeg;base64," + encodedBase64Img;

          const double fovHorizontal = M_PI * 0.33f;
          const double fovVertical = M_PI * 0.25f;

          // Add current sensor to output state
          jsonOutputState["data"]["sensors"].push_back({
            {"id", key},
            {"position", {0, 1.2, -0.5}},
            {"rotation", {0, 0, 0}},
            {"fovHorizontal", fovHorizontal},
            {"fovVertical", fovVertical},
            {"sensorTimestamp", sensorTs},
            {"imageBase64", encodedBase64Img},
          });

          // cv::imshow("Display window", img);
          // cv::waitKey(0);
        }
      }
      // TODO: do the processing for tracks

      // example track for testing
      jsonOutputState["data"]["tracks"].push_back({
        {"trackId", "0"},
        {"class", 0},
        {"position", {-5.0, 0.0, 25.0}},
        {"rotation", {0.0, 0.0, 0.0}},
        {"height", 1.5},
        {"width", 2.5},
        {"depth", 3.5},
        {"ttc", 1.0},
      });

      // Finally set the algo timestamp to the output data
      jsonOutputState["data"]["timestamp"] = _ts;

      _outputState = jsonOutputState.dump();

      // If storage is currently in "saving mode" it will save the frame, otherwise do nothing
      _storageService.saveFrame();

      // Reset the one time action commands
      _stepForward = false;
      _stepBackward = false;
      _updateTs = false;
      _jumpToTs = -1;
    }

    broadCaster.broadcast(_outputState + "\n"); // new line character to show end of message

    // Do some timing stuff and in case algo was too fast, wait for a set amount of time
    auto frameAlgoEndTime = std::chrono::high_resolution_clock::now();
    auto algoDuration = std::chrono::duration<double, std::milli>(frameAlgoEndTime - frameStartTime);
    double waitTimeMsec = (Config::goalFrameLength - 0.02) - algoDuration.count();
    if (waitTimeMsec > 0.0) {
      auto waitTimeUsec = std::chrono::microseconds(static_cast<int>(waitTimeMsec * 1000.0));
      std::this_thread::sleep_for(waitTimeUsec);
    }

    auto frameDuration = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - frameStartTime);
    // std::cout << std::fixed << std::setprecision(2) << "Frame: " << frameDuration.count() << " ms \t Algo: " << algoDuration.count() << " ms" << std::endl;
  }
}
