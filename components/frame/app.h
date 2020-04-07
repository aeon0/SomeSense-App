#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "data_reader/sensor_storage.h"
#include "com_out/irequest_listener.h"
#include "runtime_meas_service.h"
#include "types.h"
#include "output/storage.h"
#include "online_calibration/online_calibration.h"
// #include "object_detection/detector.h"
#include <signal.h>


namespace frame {
  class App : public com_out::IRequestListener {
  public:
    App(const data_reader::SensorStorage& sensorStorage, output::Storage& outputStorage, const TS& algoStartTime);

    void handleRequest(const std::string& requestType, const nlohmann::json& requestData, nlohmann::json& responseData) override;

    void run();
    void reset();

  private:
    typedef std::map<const std::string, std::shared_ptr<online_calibration::Calibrator>> CalibratorMap;

    output::Storage& _outputStorage;
    const data_reader::SensorStorage& _sensorStorage;
    // object_detection::Detector _detector;
    RuntimeMeasService _runtimeMeasService;
    CalibratorMap _calibrationMap;

    int64_t _ts; // algo timestamp of the current frame
    int _frame; // current frame counter

    const TS& _algoStartTime;
  };
}
