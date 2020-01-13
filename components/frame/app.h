#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "data_reader/sensor_storage.h"
#include "com_out/ibroadcast.h"
#include "runtime_meas_service.h"
#include "types.h"
// #include "object_detection/detector.h"
#include <signal.h>


namespace frame {
  class App {
  public:
    App(const data_reader::SensorStorage& sensorStorage, const TS& algoStartTime);

    void run(const com_out::IBroadcast& broadCaster);
    void reset();

  private:
    const data_reader::SensorStorage& _sensorStorage;
    // object_detection::Detector _detector;
    RuntimeMeasService _runtimeMeasService;

    std::string _outputState;
    int64_t _ts; // algo timestamp of the current frame
    int _frame; // current frame counter

    const TS& _algoStartTime;
  };
}
