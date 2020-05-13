#pragma once

#include <signal.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "data_reader/sensor_storage.h"
#include "com_out/irequest_listener.h"
#include "runtime_meas_service.h"
#include "types.h"
#include <atomic>
// [algos]
#include "algo/example/example.h"
#include "algo/optical_flow/optical_flow.h"


namespace frame {
  class App : public com_out::IRequestListener {
  public:
    App(const data_reader::SensorStorage& sensorStorage, serialize::AppState& appState, const TS& algoStartTime);

    // Handle requests from the outside e.g. visu
    void handleRequest(const std::string& requestType, const nlohmann::json& requestData, nlohmann::json& responseData) override;

    // Will execute runFrame() until SIGINT
    void run();
    // Execute one frame
    void runFrame();
    // Reset all internal states, in case your algo needs reseting, do this here
    void reset();

  private:
    typedef std::map<const std::string, std::shared_ptr<optical_flow::OpticalFlow>> OpticalFlowMap;

    serialize::AppState& _appState;
    const data_reader::SensorStorage& _sensorStorage;

    int64_t _ts; // algo timestamp of the current frame
    int _frame; // current frame counter

    const TS& _algoStartTime;
    std::atomic<bool> _resetEndOfFrame;

    // [algos]
    RuntimeMeasService _runtimeMeasService;
    OpticalFlowMap _opticalFlowMap;
  };
}
