#include "scheduler.h"
#include <iostream>
#include "util/proto.h"


algo::Scheduler::Scheduler(util::RuntimeMeasService& runtimeMeasService) :
  _runtimeMeasService(runtimeMeasService)
{
}

void algo::Scheduler::reset() {
  // Reset Algos
  for (auto& [key, algos]: _camAlgos) {
    algos.infer->reset();
    algos.calib->reset();
  }
}

void algo::Scheduler::exec(proto::Frame &frame) {
  _runtimeMeasService.startMeas("algo");

  for (auto camProto: frame.camsensors()) {
    auto key = camProto.key();

    // If every algo would be its own process, algos would have to do the img
    // conversion by themself, but they arent so we save a bit of runtime here
    cv::Mat cvImg;
    util::fillCvImg(cvImg, camProto.img());

    if (_camAlgos.count(key) <= 0) {
      _camAlgos.insert({key, {
        std::make_shared<algo::Inference>(_runtimeMeasService),
        std::make_shared<algo::CamCalib>(),
      }});
    }
    _camAlgos.at(key).infer->processImg(cvImg);
    _camAlgos.at(key).infer->serialize(camProto);
    _camAlgos.at(key).calib->run(camProto);
    _camAlgos.at(key).calib->serialize(camProto.mutable_calib());
  }

  _runtimeMeasService.endMeas("algo");
}
