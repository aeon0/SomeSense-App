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

    int64 getFrameFromTs = -1;
    // Check if a new frame should be created, note that pausing and stepping is only possible with recordings
    if (_outputState == "" || !_pause || !_isRecording || _stepForward || _stepBackward || _updateTs) {
      if (_isRecording) {
        // For recordings artifically set timestamp according to frame count and desired algo fps
        // depending on the commands from the player change frame and its according algo _ts
        const int maxFrames = static_cast<int>(_recLength / (Config::goalFrameLength * 1000.0));
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
          if (_frame > maxFrames) {
            // In case we have more frames than the max frames, also use the ts to get the last frame
            // otherwise fetching frames will not be successfull
            _updateTs = true;
          }
        }
        _frame = std::clamp<int>(_frame, 0, maxFrames);
        getFrameFromTs = static_cast<int64>(_frame * Config::goalFrameLength * 1000.0);

        if (getFrameFromTs >= _recLength) {
          _pause = true; // pause in case the end of the recording is reached
        }
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

      for (auto const& [key, cam]: _sensorStorage.getCams()) {
        if (!_updateTs) {
          // we can not always just use the ts as this is quite performance heavy
          getFrameFromTs = -1;
        }
        else {
          _ts = 0; // otherwise ts would not be updated
        }
        auto [success, sensorTs, img] = cam->getNewFrame(algoStartTime, getFrameFromTs);
        if (sensorTs > _ts) {
          _ts = sensorTs; // algo algo ts will be the latest sensorTs
        }

        if (success) {
          // TODO: do processing per image
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

          // If storage is currently in "saving mode" it will save the frame, otherwise do nothing
          _storageService.saveFrame();

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
