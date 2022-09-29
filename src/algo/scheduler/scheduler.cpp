#include "algo/scheduler/scheduler.h"
#include <iostream>


algo::Scheduler::Scheduler(util::RuntimeMeasService& runtimeMeasService) :
  _runtimeMeasService(runtimeMeasService)
{
  // _pointcloud = std::make_unique<pointcloud::Pointcloud>();
  // _tracker = std::make_unique<tracking::Tracker>();
}

void algo::Scheduler::reset() {
  // Reset Algos
  // for (auto& [key, opticalFlow]: _opticalFlowMap) {
  //   opticalFlow->reset();
  // }
}

void algo::Scheduler::exec(proto::Frame &frame) {
  _runtimeMeasService.startMeas("algo");

  // 1) Loop over sensor data from inputData and run sensor dependent algos
  // 1.1) Within the loop update needed sensor independent algos with this data
  // 2) Run algos which depend on multiple sensor input (if provided)
  // 3) Serialize all data from algos and create ouputData from it

  _runtimeMeasService.endMeas("algo");
}
