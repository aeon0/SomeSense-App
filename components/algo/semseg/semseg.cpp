#include "semseg.h"
#include "opencv2/highgui.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/features2d.hpp"


semseg::Semseg::Semseg(frame::RuntimeMeasService& runtimeMeasService) :
  _runtimeMeasService(runtimeMeasService) {}

void semseg::Semseg::reset() {

}

void semseg::Semseg::processImg(const cv::Mat &img) {
  std::cout << "Process Semseg Img" << std::endl;
}
