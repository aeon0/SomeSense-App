#include "app.h"
#include "config.h"
#include <iostream>
#include <thread>
#include <cmath>
#include <chrono>
#include "utilities/base64.h"
#include "utilities/json.hpp"


void frame::App::init(const std::string& sensorConfigPath) {
  _sensorStorage.initFromConfig(sensorConfigPath);
  _detector.loadModel("assets/od_model/model.onnx", "assets/od_model/prior_boxes.json");
}

void frame::App::run(const com_out::Server& server, const int& stop) {
  while (!stop) {
    const auto frameStartTime = std::chrono::high_resolution_clock::now();

    // Output State contains all data which is sent to the "outside" e.g. to visualize
    nlohmann::json jsonOutputState = {
      {"type", "server.frame"},
      {"data", {
        {"tracks", {}},
        {"sensors", {}},
      }}
    };

    for (auto const& [key, cam]: _sensorStorage.getCams()) {
      auto [ts, img] = cam->getFrame();

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
        {"imageBase64", encodedBase64Img}
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
      {"ttc", 1.0}
    });

    std::string outputState = jsonOutputState.dump();
    server.broadcast(outputState + "\n");

    // Do some timing stuff and in case algo was too fast, wait for a set amount of time
    auto frameAlgoEndTime = std::chrono::high_resolution_clock::now();
    auto algoDuration = std::chrono::duration<double, std::milli>(frameAlgoEndTime - frameStartTime);
    double waitTimeMsec = (Config::goalFrameLength - 1.0) - algoDuration.count();
    if (waitTimeMsec > 0.0) {
      auto waitTimeUsec = std::chrono::microseconds(static_cast<int>(waitTimeMsec * 1000.0));
      std::this_thread::sleep_for(waitTimeUsec);
    }
    auto frameEndTime = std::chrono::high_resolution_clock::now();
    auto frameDuration = std::chrono::duration<double, std::milli>(frameEndTime - frameStartTime);
    double deltaMsec = frameDuration.count() - Config::goalFrameLength;
    if (deltaMsec > 10.0) {
      std::cout << "WARNING: Frame too long by " << deltaMsec << " ms, (Algo: " << algoDuration.count() << " ms)" << std::endl;
    }
  }
}
