#include "scheduler.h"
#include <iostream>
#include "util/proto.h"


algo::Scheduler::Scheduler(util::RuntimeMeasService& runtimeMeasService) :
  _runtimeMeasService(runtimeMeasService)
{
  // _pointcloud = std::make_unique<pointcloud::Pointcloud>();
  // _tracker = std::make_unique<tracking::Tracker>();
}

void algo::Scheduler::reset() {
  // Reset Algos
  for (auto& [key, inf]: _inference) {
    inf->reset();
  }
}

void algo::Scheduler::exec(proto::Frame &frame) {
  _runtimeMeasService.startMeas("algo");
  // Loop over sensor data from inputData and run sensor dependent algos
  for (auto camProto: frame.camsensors()) {
    auto key = camProto.key();

    cv::Mat cvImg;
    util::fillCvImg(cvImg, camProto.img());

    // Do inference
    if (_inference.count(key) <= 0) {
      _inference.insert({key, std::make_unique<algo::Inference>(_runtimeMeasService)});
    }
    _inference.at(key)->processImg(cvImg);
    _inference.at(key)->serialize(camProto);
  }

  // 1.1) Within the loop update needed sensor independent algos with this data
  // 2) Run algos which depend on multiple sensor input (if provided)
  // 3) Serialize all data from algos and create ouputData from it

  _runtimeMeasService.endMeas("algo");
}
