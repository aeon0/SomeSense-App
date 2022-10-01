#include "scheduler.h"
#include <iostream>
#include "util/proto.h"


algo::Scheduler::Scheduler(util::RuntimeMeasService& runtimeMeasService) :
  _runtimeMeasService(runtimeMeasService)
{
  _pointcloud = std::make_unique<algo::Pointcloud>(runtimeMeasService);
}

void algo::Scheduler::reset() {
  for (auto& [key, algos]: _camAlgos) {
    algos.infer->reset();
    algos.calib->reset();
  }
  _pointcloud->reset();
}

void algo::Scheduler::exec(proto::Frame &frame) {
  _runtimeMeasService.startMeas("algo");

  for (int i = 0; i < frame.camsensors_size(); ++i) {
    auto camProto = frame.mutable_camsensors(i);
    auto key = camProto->key();

    if (_camAlgos.count(key) <= 0) {
      _camAlgos.insert({key, {
        std::make_shared<algo::Inference>(_runtimeMeasService),
        std::make_shared<algo::CamCalib>(),
      }});
    }
    _camAlgos.at(key).infer->run(camProto->img());
    _camAlgos.at(key).infer->serialize(camProto);

    _camAlgos.at(key).calib->run(camProto);
    _camAlgos.at(key).calib->serialize(camProto->mutable_calib());
  }

  _pointcloud->run(frame);
  _pointcloud->serialize(frame);

  _runtimeMeasService.endMeas("algo");
}
