#pragma once

#include <signal.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "util/runtime_meas_service.h"
#include "frame.pb.h"
// [algos]
#include "algo/inference/inference.h"
#include "algo/cam_calib/cam_calib.h"


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

    // [algos] per cam sensor
    struct CamAlgoTypes {
      std::shared_ptr<algo::Inference> infer;
      std::shared_ptr<algo::CamCalib> calib;
    };
    std::map<const std::string, CamAlgoTypes> _camAlgos;
  };
}
