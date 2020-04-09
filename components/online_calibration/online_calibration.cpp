#include "online_calibration.h"
#include "opencv2/highgui.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/features2d.hpp"


online_calibration::Calibrator::Calibrator(frame::RuntimeMeasService& runtimeMeasService) :
  _runtimeMeasService(runtimeMeasService) {}

void online_calibration::Calibrator::reset() {

}

