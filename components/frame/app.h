#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "data_reader/sensor_storage.h"
#include "data_reader/rec_storage.h"
#include "com_out/ibroadcast.h"
#include "com_out/irequest_listener.h"
#include "object_detection/detector.h"
#include <signal.h>


namespace frame {
  class App: public com_out::IRequestListener {
  public:
    App(const std::string& sensorConfigPath);

    void run(const com_out::IBroadcast& broadCaster);
    void handleRequest(const std::string& requestType, const nlohmann::json& requestData, nlohmann::json& responseData) override;

  private:
    data_reader::RecStorage _recStorage;
    data_reader::SensorStorage _sensorStorage;
    object_detection::Detector _detector;

    // Actions to take for recordings
    bool _pause;
    bool _stepForward;
    bool _stepBackward;
    bool _updateTs;
    bool _record;

    int64 _jumpToTs;
    bool _isRecording;
    int64 _recLength;

    // Current output state sent to the clients
    std::string _outputState;
    int64 _ts; // algo timestamp of the current frame
    int _frame; // current frame counter
  };
}
