#include "app.h"
#include "config.h"
#include <iostream>
#include <thread>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <memory>
#include "utilities/json.hpp"
#include "output/types.h"

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

volatile sig_atomic_t stopFromSignal = 0;
void sighandler(int signum) { stopFromSignal = 1; }


frame::App::App(const data_reader::SensorStorage& sensorStorage, output::Storage& outputStorage, const TS& algoStartTime) :
  _sensorStorage(sensorStorage), _outputStorage(outputStorage), _algoStartTime(algoStartTime),
  _ts(-1), _frame(-1), _runtimeMeasService(algoStartTime) {
  // Listen to SIGINT (usually ctrl + c on terminal) to stop endless algo loop
  signal(SIGINT, &sighandler);

  // _detector.loadModel("assets/od_model/model.onnx", "assets/od_model/prior_boxes.json");
}

void frame::App::handleRequest(const std::string& requestType, const nlohmann::json& requestData, nlohmann::json& responseData) {
  if (requestData["type"] == "client.step_backward" || requestData["type"] == "client.jump_to_ts") {
    // TODO: Frame is reset to -1, but it should be reset to whatever we jump to...
    reset();
  }
}

void frame::App::reset() {
  _ts = -1;
  _frame = -1;

  for (auto [key, opticalFlow]: _opticalFlowMap) {
    opticalFlow.reset();
  }
}

void frame::App::run() {
  while (!stopFromSignal) {
    
  }
}

void frame::App::runFrame() {
  _runtimeMeasService.startMeas("algo");

  // Timepoint of frame start
  const auto frameStartTime = std::chrono::high_resolution_clock::now();
  // To have a constant framerate, we would wait until this time in case the algo was quicker
  const auto plannedFrameEndTime = frameStartTime + std::chrono::duration<double, std::micro>(Config::goalFrameLength);
  // Ts (from app start) of frame start. This Ts is not the algo Ts. The sensor Ts determine the algo Ts
  const auto frameStartTS = static_cast<int64_t>(std::chrono::duration<double, std::micro>(frameStartTime - _algoStartTime).count());
  
  // Output State contains all data which is sent to the "outside" e.g. to visualize
  output::Frame frameData;

  const int64_t previousTs = _ts;
  int sensorIdx = 0;
  bool gotNewSensorData = false;

  for (auto const& [key, cam]: _sensorStorage.getCams()) {
    _runtimeMeasService.startMeas("read_img_" + key);
    auto [success, sensorTs, img] = cam->getFrame();
    _runtimeMeasService.endMeas("read_img_" + key);

    if (success && img.size().width > 0 && img.size().height > 0 && sensorTs > previousTs) {
      // Create grayscale img
      cv::Mat grayScaleImg;
      cv::cvtColor(img, grayScaleImg, cv::COLOR_BGR2GRAY);

      // Do processing per 2D image
      // _runtimeMeasService.startMeas("detect_" + key);
      // _detector.detect(img);
      // _runtimeMeasService.endMeas("detect_" + key);

      // _runtimeMeasService.startMeas("optical_flow_" + key);
      // if (_opticalFlowMap.count(key) <= 0) {
      //   _opticalFlowMap.insert({key, std::make_shared<optical_flow::OpticalFlow>(_runtimeMeasService)});
      // }
      // _opticalFlowMap.at(key)->update(grayScaleImg, sensorTs);
      // _runtimeMeasService.endMeas("optical_flow_" + key);

      // Set image to output state
      _runtimeMeasService.startMeas("img_to_output" + key);
      output::CamImg camImgData {
        sensorIdx,
        sensorTs,
        img.clone(),
        img.size().width,
        img.size().height,
        img.channels()
      };
      _outputStorage.setCamImg(key, camImgData);
      _runtimeMeasService.endMeas("img_to_output" + key);

      gotNewSensorData = true;
      if (sensorTs > _ts) {
        _ts = sensorTs; // take latest sensorTs as as algoTs
      }
    }

    // Add sensor to outputstate
    frameData.camSensors.push_back({
      sensorIdx,
      key,
      {cam->getFocalX(), cam->getFocalY()}, // focal length
      {cam->getPrincipalPointX(), cam->getPrincipalPointY()}, // principal point
      {0, 1.2, -0.5}, // position: x, y, z
      {0, 0, 0}, // rotation: yaw, pitch, roll
      cam->getHorizontalFov(),
      cam->getVerticalFov()});

    sensorIdx++;
  }

  if (gotNewSensorData) {
    // TODO: do the processing for tracks
    // Add example track for testing
    // frameData.tracks.push_back({"0", 0, {-5.0, 0.0, 25.0}, {0.0, 0.0, 0.0}, 0, 1.5, 2.5, 3.5, 0.0});

    _runtimeMeasService.endMeas("algo");

    // Finally set the algo timestamp to the output data
    frameData.timestamp = _ts;
    frameData.frameCount = _frame;
    frameData.frameStart = frameStartTS;
    frameData.plannedFrameLength = Config::goalFrameLength;
    // Add time measurements to the json output
    auto runtimeMeas = _runtimeMeasService.serializeMeas();
    frameData.runtimeMeas.assign(runtimeMeas.begin(), runtimeMeas.end());

    _outputStorage.set(frameData);

    _frame++;

    // Complete Algo duration for debugging
    // const auto endTime = std::chrono::high_resolution_clock::now();
    // const auto durAlgo = std::chrono::duration<double, std::milli>(endTime - frameStartTime);
    // std::cout << durAlgo.count() << std::endl;
  }

  _runtimeMeasService.reset();
  // Wait till end of frame in case algo was quicker too keep consistent algo frame rate
  std::this_thread::sleep_until(plannedFrameEndTime);
}
