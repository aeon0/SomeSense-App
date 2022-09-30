#pragma once

#include <signal.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "util/runtime_meas_service.h"
#include "frame.pb.h"

// [algos]
// #include "algo/optical_flow/optical_flow.h"
// #include "algo/inference/inference.h"
// #include "algo/cam_calib/cam_calib.h"
// #include "algo/pointcloud/pointcloud.h"
// #include "algo/tracking/tracker.h"
// #include "algo/example/example.h"


namespace algo {
  class Scheduler {
  public:
    // TODO: pass some sort of sensor config to create sensor dependent algos
    Scheduler(util::RuntimeMeasService& runtimeMeasService);

    // Exec all algos in a specified order depending on inputData
    void exec(proto::Frame& frame);
    // Reset all internal states, in case your algo needs reseting, do this here
    void reset();

  private:
    util::RuntimeMeasService& _runtimeMeasService;

    // // [algos] per sensor
    // std::map<const std::string, std::unique_ptr<optical_flow::OpticalFlow>> _opticalFlowMap; // optical flow per cam sensor
    // std::map<const std::string, std::unique_ptr<inference::Inference>> _inference; // inference per cam sensor
    // std::map<const std::string, std::unique_ptr<cam_calib::CamCalib>> _camCalib; // cam_calib per cam sensor
    // // [algos] sensor independent
    // std::unique_ptr<pointcloud::Pointcloud> _pointcloud;
    // std::unique_ptr<tracking::Tracker> _tracker;
  };
}
