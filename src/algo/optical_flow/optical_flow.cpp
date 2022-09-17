#include "optical_flow.h"
#include "opencv2/highgui.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/features2d.hpp"


optical_flow::OpticalFlow::OpticalFlow(frame::RuntimeMeasService& runtimeMeasService) :
  _runtimeMeasService(runtimeMeasService)
{

}

void optical_flow::OpticalFlow::reset() {

}

void optical_flow::OpticalFlow::processImg(const cv::Mat &img, const int64_t ts) {

}

void optical_flow::OpticalFlow::serialize(CapnpOutput::CamSensor::OpticalFlow::Builder& builder) {
  // builder.setEndTs(_prevTs);
  // builder.setDeltaTime(_deltaTime);

  // auto flowTracks = builder.initFlowTracks(_flow.size());
  // for (int i = 0; i < _flow.size(); ++i) {
  //   flowTracks[i].setStartX(_flow[i].first.x);
  //   flowTracks[i].setStartY(_flow[i].first.y);
  //   flowTracks[i].setEndY(_flow[i].second.y);
  //   flowTracks[i].setEndY(_flow[i].second.y);
  // }
}
