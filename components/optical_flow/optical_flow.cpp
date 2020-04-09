#include "optical_flow.h"
#include "opencv2/highgui.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/features2d.hpp"


optical_flow::OpticalFlow::OpticalFlow(frame::RuntimeMeasService& runtimeMeasService) :
  _runtimeMeasService(runtimeMeasService), _framesSinceRefresh(REFRESH_AFTER) {
    _framesSinceRefresh = REFRESH_AFTER;
  }

void optical_flow::OpticalFlow::reset() {
  _featuresLastFrame.clear();
}

void optical_flow::OpticalFlow::update(const cv::Mat &img, const int64_t ts) {
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

    // Show image for Debugging
    cv::Mat debugImg;
    cv::cvtColor(img, debugImg, cv::COLOR_GRAY2BGR);

    for (int i = 0; i < _featuresLastFrame.size(); ++i) {
      cv::circle(debugImg, _featuresLastFrame[i], 2, {255, 0, 0});
      cv::circle(debugImg, newPts[i], 2, {0, 255, 0});
      cv::arrowedLine(debugImg, _featuresLastFrame[i], newPts[i], {0, 0, 255});
    }

    cv::namedWindow("Debug Calib Window", cv::WINDOW_AUTOSIZE);
    cv::imshow("Debug Calib Window", debugImg);
    cv::waitKey(1);
  }

  _runtimeMeasService.startMeas("find_good_features");
  // As find good features takes quite some runtime, only use it every 3 frames
  if (_framesSinceRefresh++ >= REFRESH_AFTER) {
    cv::goodFeaturesToTrack(img, _featuresLastFrame, 400, 0.05, 10);
    _framesSinceRefresh = 0;
  }
  _runtimeMeasService.endMeas("find_good_features");
  img.copyTo(_prevImg);

  _runtimeMeasService.printToConsole();
}

