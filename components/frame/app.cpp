#include "app.h"
#include "config.h"
#include <iostream>
#include <thread>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <memory>
#include <kj/common.h>
#include "utilities/json.hpp"
#include <capnp/message.h>
#include "serialize/frame.capnp.h"

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

volatile sig_atomic_t stopFromSignal = 0;
void sighandler(int signum) { stopFromSignal = 1; }


frame::App::App(const data_reader::SensorStorage& sensorStorage, serialize::AppState& appState, const TS& algoStartTime) :
  _sensorStorage(sensorStorage), _appState(appState), _algoStartTime(algoStartTime),
  _ts(-1), _frame(0), _runtimeMeasService(algoStartTime) {
  // Listen to SIGINT (usually ctrl + c on terminal) to stop endless algo loop
  signal(SIGINT, &sighandler);

  _pointcloud = std::make_unique<pointcloud::Pointcloud>(_runtimeMeasService);
  _tracker = std::make_unique<tracking::Tracker>(_runtimeMeasService);
}

void frame::App::handleRequest(const std::string& requestType, const nlohmann::json& requestData, nlohmann::json& responseData) {
  if (requestData["type"] == "client.reset_algo") {
    _resetEndOfFrame = true;
  }
}

void frame::App::reset() {
  for (auto& [key, opticalFlow]: _opticalFlowMap) {
    opticalFlow->reset();
  }

  _ts = -1;
  _frame = 0;
}

void frame::App::run() {
  while (!stopFromSignal) {
    runFrame();
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

  // Create Capnp message which will be saved into the output state to be sent to clients by the server
  auto messagePtr = std::make_unique<capnp::MallocMessageBuilder>();
  auto capnpFrameData = messagePtr->initRoot<CapnpOutput::Frame>();
  auto capnpCamSensors = capnpFrameData.initCamSensors(_sensorStorage.getCams().size());

  const int64_t previousTs = _ts;
  int camSensorIdx = 0;

  // Loop through cameras and do 2D processing on them
  for (auto const [key, cam]: _sensorStorage.getCams()) {
    _runtimeMeasService.startMeas("read_img_" + key);
    auto [success, sensorTs, img] = cam->getFrame();
    _runtimeMeasService.endMeas("read_img_" + key);

    if (success && img.size().width > 0 && img.size().height > 0 && sensorTs > previousTs) {
      // Tell algo that at least one sensor provided new data
      if (sensorTs > _ts) {
        _ts = sensorTs; // take latest sensorTs as as algoTs
      }

      // Create grayscale img
      cv::Mat grayScaleImg;
      cv::cvtColor(img, grayScaleImg, cv::COLOR_BGR2GRAY);

      auto camSensorBuilder = capnpCamSensors[camSensorIdx];

      // Call [algos] on 2D image
      // Optical Flow - currently a empty hull
      if (_opticalFlowMap.count(key) <= 0) {
        _opticalFlowMap.insert({key, std::make_unique<optical_flow::OpticalFlow>(_runtimeMeasService)});
      }
      _opticalFlowMap.at(key)->processImg(grayScaleImg, sensorTs);
      auto opticalFlowBuilder = camSensorBuilder.getOpticalFlow();
      _opticalFlowMap.at(key)->serialize(opticalFlowBuilder);

      // Inference on EdgeTpu
      if (_inference.count(key) <= 0) {
        _inference.insert({key, std::make_unique<inference::Inference>(_runtimeMeasService)});
      }
      _inference.at(key)->processImg(img);
      _inference.at(key)->serialize(camSensorBuilder);

      // Cam calibration
      if (_camCalib.count(key) <= 0) {
        _camCalib.insert({key, std::make_unique<cam_calib::CamCalib>(_runtimeMeasService, *cam)});
      }
      _camCalib.at(key)->calibrate(_inference.at(key)->getSemseg(), _inference.at(key)->getDepth(), _inference.at(key)->getRoi());

      cam->serialize(camSensorBuilder, camSensorIdx, sensorTs, img);

      // Update sensor independent algos
      _pointcloud->processData(_inference.at(key)->getSemseg(), _inference.at(key)->getDepth(), _inference.at(key)->getRoi(), *cam);
      _tracker->update(_inference.at(key)->getObjects2D(), *cam);
    }

    camSensorIdx++;
  }

  // TODO: In case there is no sensor input for a bit, we still should update (and thus predict) the tracker
  if (_ts > previousTs) {
    _pointcloud->serialize(capnpFrameData);
    _tracker->serialize(capnpFrameData);

    // Finally set the algo timestamp to the output data
    capnpFrameData.setFrameCount(_frame);
    capnpFrameData.setFrameStart(frameStartTS);
    capnpFrameData.setPlannedFrameLength(Config::goalFrameLength);
    capnpFrameData.setTimestamp(_ts);

    _runtimeMeasService.endMeas("algo");
    _runtimeMeasService.serialize(capnpFrameData);

    _appState.setFrame(std::move(messagePtr));
    _frame++;
  }

  _runtimeMeasService.reset();
  if (_resetEndOfFrame) {
    reset();
    _resetEndOfFrame = false;
  }
  // keep consistent algo framerate
  std::this_thread::sleep_until(plannedFrameEndTime);
}
