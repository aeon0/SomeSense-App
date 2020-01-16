#include "app.h"
#include "config.h"
#include <iostream>
#include <thread>
#include <cmath>
#include <chrono>
#include <algorithm>
#include "utilities/json.hpp"
#include "com_out/output_state.h"

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

volatile sig_atomic_t stopFromSignal = 0;
void sighandler(int signum) { stopFromSignal = 1; }


frame::App::App(const data_reader::SensorStorage& sensorStorage, const TS& algoStartTime) :
  _sensorStorage(sensorStorage), _algoStartTime(algoStartTime), _ts(0), _outputState(""), _frame(-1), _runtimeMeasService(algoStartTime) {
  // Listen to SIGINT (usually ctrl + c on terminal) to stop endless algo loop
  signal(SIGINT, &sighandler);

  // _detector.loadModel("assets/od_model/model.onnx", "assets/od_model/prior_boxes.json");
}

void reset() {
  std::cout << "Reset Algo..." << std::endl;
}

void frame::App::run(const com_out::IBroadcast& broadCaster) {
  while (!stopFromSignal) {
    _runtimeMeasService.startMeas("algo");

    const auto frameStartTime = std::chrono::high_resolution_clock::now();
    const auto plannedFrameEndTime = frameStartTime + std::chrono::microseconds(Config::goalFrameLength);

    // Output State contains all data which is sent to the "outside" e.g. to visualize
    com_out::OutputState outputState;

    const int64_t previousTs = _ts;
    _ts = 0;
    int sensorIdx = 0;
    bool gotNewSensorData = false;

    for (auto const& [key, cam]: _sensorStorage.getCams()) {
      _runtimeMeasService.startMeas("get_img_" + key);
      auto [success, sensorTs, img] = cam->getFrame();
      _runtimeMeasService.endMeas("get_img_" + key);

      if (success && img.size().width > 0 && img.size().height > 0 && sensorTs > previousTs) {
        // Do processing per image
        // _detector.detect(img);

        _runtimeMeasService.startMeas("data_proc_" + key);

        // Resize image to a reasonable size
        // Note: Be aware that the resize should always be some integer factor, e.g. 640 -> 320
        //       otherwise runtime can be quite high on the resize
        cv::Mat outImg;
        cv::Size outSize;
        outSize.width = Config::outImgWidth;
        const double scaleFactor = static_cast<double>(outSize.width) / static_cast<double>(img.size().width);
        outSize.height = img.size().height * scaleFactor;
        cv::resize(img, outImg, outSize, 0.0, 0.0, cv::InterpolationFlags::INTER_NEAREST);
        
        // broadCaster.broadcast(sensorIdx, outImg.data, outImg.size().width, outImg.size().height, outImg.channels(), sensorTs);

        gotNewSensorData = true;
        if (sensorTs > _ts) {
          _ts = sensorTs; // take latest sensorTs as as algoTs
        }

        _runtimeMeasService.endMeas("data_proc_" + key);
      }

      // Add sensor to outputstate
      outputState.sensors.push_back({sensorIdx, key, {0, 1.2, -0.5}, {0, 0, 0}, (M_PI * 0.33), (M_PI * 0.25)});

      sensorIdx++;
    }

    if (gotNewSensorData) {
      // TODO: do the processing for tracks

      _runtimeMeasService.endMeas("algo");
      _runtimeMeasService.startMeas("send_data");

      // Add example track for testing
      outputState.tracks.push_back({"0", 0, {-5.0, 0.0, 25.0}, {0.0, 0.0, 0.0}, 0, 1.5, 2.5, 3.5, 0.0});

      // Finally set the algo timestamp to the output data
      outputState.timestamp = _ts;
      outputState.frame = _frame;
      // Add time measurements to the json output
      auto runtimeMeas = _runtimeMeasService.serializeMeas();
      outputState.runtimeMeas.assign(runtimeMeas.begin(), runtimeMeas.end());
      
      // broadCaster.broadcast(_outputState);

      _runtimeMeasService.endMeas("send_data");

      _frame++;
    }

    // _runtimeMeasService.printToConsole();
    _runtimeMeasService.reset();
    // Wait till end of frame in case algo was quicker too keep consistent algo frame rate
    std::this_thread::sleep_until(plannedFrameEndTime);
  }
}
