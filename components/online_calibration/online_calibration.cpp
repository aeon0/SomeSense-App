#include "online_calibration.h"
#include "opencv2/highgui.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/features2d.hpp"


online_calibration::Calibrator::Calibrator(frame::RuntimeMeasService& runtimeMeasService) :
  _runtimeMeasService(runtimeMeasService) {}

void online_calibration::Calibrator::reset() {
  _featuresLastFrame.clear();
}

void online_calibration::Calibrator::calibrate(const cv::Mat &img) {
  // if (_featuresLastFrame.size() > 0) {
  if (_prevImg.size().width > 0 && _prevImg.size().height > 0) {
    _runtimeMeasService.startMeas("calc_flow");
    std::vector<uchar> status;
    std::vector<float> err;
    std::vector<cv::Point2f> newPts;
    cv::calcOpticalFlowPyrLK(
      _prevImg, img,
      _featuresLastFrame, newPts,
      status,
      err,
      cv::Size(13, 13),
      3,
      cv::TermCriteria((cv::TermCriteria::COUNT) + (cv::TermCriteria::EPS), 30, 0.01));
    _runtimeMeasService.endMeas("calc_flow");
    // cv::Mat debugImg;
    // cv::cvtColor(img, debugImg, cv::COLOR_GRAY2BGR);

    // for (int i = 0; i < _featuresLastFrame.size(); ++i) {
    //   cv::circle(debugImg, _featuresLastFrame[i], 2, {255, 0, 0});
    //   cv::circle(debugImg, newPts[i], 2, {0, 255, 0});
    //   cv::arrowedLine(debugImg, _featuresLastFrame[i], newPts[i], {0, 255, 0});
    // }

    // cv::namedWindow("Debug Calib Window", cv::WINDOW_AUTOSIZE);
    // cv::imshow("Debug Calib Window", debugImg);
    // cv::waitKey(1);
  }
  _runtimeMeasService.startMeas("find_good_features");
  // TODO: This is taking up quite some runtime, dont do this every frame!
  cv::goodFeaturesToTrack(img, _featuresLastFrame, 100, 0.1, 10);
  _runtimeMeasService.endMeas("find_good_features");
  img.copyTo(_prevImg);
}

