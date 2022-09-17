#include "algo/scheduler/scheduler.h"
#include <iostream>


algo::Scheduler::Scheduler(const TS& algoStartTime) : 
  _runtimeMeasService(algoStartTime)
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

void algo::Scheduler::exec(int inputData, int& outputData) {
  _runtimeMeasService.startMeas("algo");

  // 1) Loop over sensor data from inputData and run sensor dependent algos
  // 1.1) Within the loop update needed sensor independent algos with this data
  // 2) Run algos which depend on multiple sensor input (if provided)
  // 3) Serialize all data from algos and create ouputData from it

  outputData = 3;

  _runtimeMeasService.endMeas("algo");
  _runtimeMeasService.printToConsole();
  _runtimeMeasService.reset();
}
