#include "app.h"
#include "config.h"
#include <iostream>
#include <thread>
#include <cmath>
#include <chrono>
#include "utilities/base64.h"
#include "utilities/json.hpp"

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

volatile sig_atomic_t stopFromSignal = 0;
void sighandler(int signum) { stopFromSignal = 1; }


void frame::App::init(const std::string& sensorConfigPath) {
  // Listen to SIGINT (usually ctrl + c on terminal), has to be after the server thread!
  signal(SIGINT, &sighandler);

  _sensorStorage.initFromConfig(sensorConfigPath);
  _detector.loadModel("assets/od_model/model.onnx", "assets/od_model/prior_boxes.json");

  _pause = true;
  _outputState = "";
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
}

void frame::App::run(const com_out::Server& server) {
  while (!stopFromSignal) {
    const auto frameStartTime = std::chrono::high_resolution_clock::now();

    bool isRecording = false;
    int64 recLength = 0;
    // Check if any of the sensors is playing from a recording
    for (auto const& [key, cam]: _sensorStorage.getCams()) {
      if (cam->isRecording()) {
        isRecording = true;
        if (cam->getRecLength() > recLength) {
          recLength = cam->getRecLength();
        }
      }
    }

    // pausing is only possible for recordings
    if (_outputState == "" || !_pause || !isRecording) {
      // Output State contains all data which is sent to the "outside" e.g. to visualize
      nlohmann::json jsonOutputState = {
        {"type", "server.frame"},
        {"data", {
          {"tracks", {}},
          {"sensors", {}},
          {"timestamp", 0},
          {"isRecording", isRecording},
          {"recLength", recLength},
          {"isPlaying", _pause},
        }}
      };

      for (auto const& [key, cam]: _sensorStorage.getCams()) {
        auto [ts, img] = cam->getFrame();
        jsonOutputState["data"]["timestamp"] = ts; // TODO: Remove this, but for now there is no algo info, lets use the image timestamp

        // TODO: do the whole image processing stuff
        _detector.detect(img);

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
          {"imageBase64", encodedBase64Img},
        });

        // cv::imshow("Display window", img);
        // cv::waitKey(0);
      }
      // Loop through other sensor types if needed and do the processing

      // TODO: input all data to tracker

      // example track for testing
      jsonOutputState["data"]["tracks"].push_back({
        {"trackId", "0"},
        {"class", 0},
        {"position", {5.0, 0.0, 10.0}},
        {"rotation", {0.0, 0.0, 0.0}},
        {"height", 1.5},
        {"width", 0.5},
        {"depth", 3.0},
        {"ttc", 1.0},
      });

      _outputState = jsonOutputState.dump();
    }

    server.broadcast(_outputState + "\n"); // new line character to show end of message

    // Do some timing stuff and in case algo was too fast, wait for a set amount of time
    auto frameAlgoEndTime = std::chrono::high_resolution_clock::now();
    auto algoDuration = std::chrono::duration<double, std::milli>(frameAlgoEndTime - frameStartTime);
    double waitTimeMsec = (Config::goalFrameLength - 1.0) - algoDuration.count();
    if (waitTimeMsec > 0.0) {
      auto waitTimeUsec = std::chrono::microseconds(static_cast<int>(waitTimeMsec * 1000.0));
      std::this_thread::sleep_for(waitTimeUsec);
    }

    auto frameDuration = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - frameStartTime);
    // std::cout << std::fixed << std::setprecision(2) << "Frame: " << frameDuration.count() << " ms \t Algo: " << algoDuration.count() << " ms" << std::endl;
  }
}
