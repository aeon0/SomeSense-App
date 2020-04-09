#include "optical_flow.h"
#include "opencv2/highgui.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/features2d.hpp"


optical_flow::OpticalFlow::OpticalFlow(frame::RuntimeMeasService& runtimeMeasService) :
  _runtimeMeasService(runtimeMeasService), _framesSinceRefresh(REFRESH_AFTER) {}

void optical_flow::OpticalFlow::reset() {
  _prevFreatures.clear();
}

void optical_flow::OpticalFlow::update(const cv::Mat &img, const int64_t ts) {
  std::vector<cv::Point2f> newFeatures;

  if (_prevFreatures.size() > 0 && _prevImg.size().height > 0 && _prevImg.size().width > 0) {
    // Calc delta time to last frame
    _deltaTime = (ts - _prevTs) * 0,001; // convert from [us] to [ms]

    _runtimeMeasService.startMeas("calc_flow");
    std::vector<uchar> status;
    std::vector<float> err;
    cv::calcOpticalFlowPyrLK(
      _prevImg, img,
      _prevFreatures, newFeatures,
      status,
      err,
      cv::Size(13, 13),
      3,
      cv::TermCriteria((cv::TermCriteria::COUNT) + (cv::TermCriteria::EPS), 30, 0.01));
    _runtimeMeasService.endMeas("calc_flow");

    // Find fundamental matrix to filter out flow vectors
    // TODO: Points which are within a object should be ignored for this (one object detection works)
    _fundamentalMat = cv::findFundamentalMat(_prevFreatures, newFeatures, cv::Mat(), cv::FM_RANSAC, 3.0, 0.99);

    // Save optical flow to internal flow
    _flow.clear();
    for (int i = 0; i < _prevFreatures.size(); ++i) {
      _flow.push_back(std::make_pair(_prevFreatures[i], newFeatures[i]));
    }

    // Show image for Debugging
    cv::Mat debugImg;
    cv::cvtColor(img, debugImg, cv::COLOR_GRAY2BGR);
    for (int i = 0; i < _prevFreatures.size(); ++i) {
      cv::circle(debugImg, _prevFreatures[i], 2, {255, 0, 0});
      cv::circle(debugImg, newFeatures[i], 2, {0, 255, 0});
      cv::arrowedLine(debugImg, _prevFreatures[i], newFeatures[i], {0, 0, 255});
    }
    cv::namedWindow("Debug Optical Flow Window", cv::WINDOW_AUTOSIZE);
    cv::imshow("Debug Optical Flow", debugImg);
    cv::waitKey(1);
  }

  _runtimeMeasService.startMeas("find_good_features");
  // Find good features takes quite some runtime, only use it every x frames
  if (_framesSinceRefresh >= (REFRESH_AFTER - 1)) {
    cv::goodFeaturesToTrack(img, _prevFreatures, 400, 0.05, 10);
    _framesSinceRefresh = 0;
  }
  else {
    _framesSinceRefresh++;
    _prevFreatures = newFeatures;
  }
  _runtimeMeasService.endMeas("find_good_features");
  
  // Save data for next frame
  img.copyTo(_prevImg);
  _prevTs = ts;

  _runtimeMeasService.printToConsole();
}
