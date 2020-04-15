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
#include "output/frame.capnp.h"

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
  bool gotNewSensorData = false;

  // Loop through cameras and do 2D processing on them
  for (auto const& [key, cam]: _sensorStorage.getCams()) {
    _runtimeMeasService.startMeas("read_img_" + key);
    auto [success, sensorTs, img] = cam->getFrame();
    _runtimeMeasService.endMeas("read_img_" + key);

    if (success && img.size().width > 0 && img.size().height > 0 && sensorTs > previousTs) {
      // Create grayscale img
      cv::Mat grayScaleImg;
      cv::cvtColor(img, grayScaleImg, cv::COLOR_BGR2GRAY);

      // _runtimeMeasService.startMeas("detect_" + key);
      // _detector.detect(img);
      // _runtimeMeasService.endMeas("detect_" + key);

      // _runtimeMeasService.startMeas("optical_flow_" + key);
      // if (_opticalFlowMap.count(key) <= 0) {
      //   _opticalFlowMap.insert({key, std::make_shared<optical_flow::OpticalFlow>(_runtimeMeasService)});
      // }
      // _opticalFlowMap.at(key)->update(grayScaleImg, sensorTs);
      // _runtimeMeasService.endMeas("optical_flow_" + key);

      gotNewSensorData = true;
      if (sensorTs > _ts) {
        _ts = sensorTs; // take latest sensorTs as as algoTs
      }

      // Add sensor to outputstate
      capnpCamSensors[camSensorIdx].setIdx(camSensorIdx);
      capnpCamSensors[camSensorIdx].setTimestamp(sensorTs);
      capnpCamSensors[camSensorIdx].setKey(key);
      capnpCamSensors[camSensorIdx].setFocalLengthX(cam->getFocalX());
      capnpCamSensors[camSensorIdx].setFocalLengthY(cam->getFocalY());
      capnpCamSensors[camSensorIdx].setPrincipalPointX(cam->getPrincipalPointX());
      capnpCamSensors[camSensorIdx].setPrincipalPointY(cam->getPrincipalPointY());
      // TODO: position is not filled in autosar, also needs adaptation in visu
      capnpCamSensors[camSensorIdx].setX(0);
      capnpCamSensors[camSensorIdx].setY(1.2);
      capnpCamSensors[camSensorIdx].setZ(-0.5);
      capnpCamSensors[camSensorIdx].setYaw(0);
      capnpCamSensors[camSensorIdx].setPitch(0);
      capnpCamSensors[camSensorIdx].setRoll(0);
      capnpCamSensors[camSensorIdx].setFovHorizontal(cam->getHorizontalFov());
      capnpCamSensors[camSensorIdx].setFovVertical(cam->getVerticalFov());

      // Fill img
      capnpCamSensors[camSensorIdx].getImg().setWidth(img.size().width);
      capnpCamSensors[camSensorIdx].getImg().setHeight(img.size().height);
      capnpCamSensors[camSensorIdx].getImg().setChannels(img.channels());
      capnpCamSensors[camSensorIdx].getImg().setData(
        kj::arrayPtr(img.data, img.size().width * img.size().height * img.channels() * sizeof(uchar))
      );
    }

    camSensorIdx++;
  }

  if (gotNewSensorData) {
    // TODO: do the processing for tracks
    auto capnpTracks = capnpFrameData.initTracks(0);

    _runtimeMeasService.endMeas("algo");

    // Finally set the algo timestamp to the output data
    capnpFrameData.setFrameCount(_frame);
    capnpFrameData.setFrameStart(frameStartTS);
    capnpFrameData.setPlannedFrameLength(Config::goalFrameLength);
    capnpFrameData.setTimestamp(_ts);

    // Add runtime meas
    auto runtimeMeasData = _runtimeMeasService.getAllMeas();
    auto capnpRuntimeMeas = capnpFrameData.initRuntimeMeas(runtimeMeasData.size());
    int i = 0;
    for (auto [key, val]: runtimeMeasData) {
      auto startMeasTs = static_cast<long int>(std::chrono::duration<double, std::micro>(val.startTime - _algoStartTime).count());
      capnpRuntimeMeas[i].setName(key);
      capnpRuntimeMeas[i].setDuration(val.duration.count());
      capnpRuntimeMeas[i].setStart(startMeasTs);
      i++;
    }

    _outputStorage.set(std::move(messagePtr));
    _frame++;

    // Complete Algo duration for debugging
    // const auto endTime = std::chrono::high_resolution_clock::now();
    // const auto durAlgo = std::chrono::duration<double, std::milli>(endTime - frameStartTime);
    // std::cout << durAlgo.count() << std::endl;
  }

  _runtimeMeasService.reset();
  // keep consistent algo framerate
  std::this_thread::sleep_until(plannedFrameEndTime);
}
