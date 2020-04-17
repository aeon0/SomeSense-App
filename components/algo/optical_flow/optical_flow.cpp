#include "optical_flow.h"
#include "opencv2/highgui.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/features2d.hpp"


optical_flow::OpticalFlow::OpticalFlow(frame::RuntimeMeasService& runtimeMeasService) :
  _runtimeMeasService(runtimeMeasService), _refreshAfter(3), _framesSinceRefresh(3) {}

void optical_flow::OpticalFlow::reset() {
  _prevFreatures.clear();
}

void optical_flow::OpticalFlow::update(const cv::Mat &img, const int64_t ts) {
  std::vector<cv::Point2f> newFeatures;

  if (_prevFreatures.size() > 0 && _prevImg.size().height > 0 && _prevImg.size().width > 0) {
    // Calc delta time to last frame
    _deltaTime = (ts - _prevTs) * 0.001; // convert from [us] to [ms]

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


    // Save optical flow to internal flow
    _flow.clear();
    for (int i = 0; i < _prevFreatures.size(); ++i) {
      _flow.push_back(std::make_pair(_prevFreatures[i], newFeatures[i]));
    }

    // Show image for Debugging
    // cv::Mat debugImg;
    // cv::cvtColor(img, debugImg, cv::COLOR_GRAY2BGR);
    // for (auto f: _flow) {
    //   cv::circle(debugImg, f.first, 2, {255, 0, 0});
    //   cv::circle(debugImg, f.second, 2, {0, 255, 0});
    //   cv::arrowedLine(debugImg, f.first, f.second, {0, 0, 255});
    // }
    // cv::namedWindow("Debug Optical Flow Window", cv::WINDOW_AUTOSIZE);
    // cv::imshow("Debug Optical Flow", debugImg);
    // cv::waitKey(1);
  }

  _runtimeMeasService.startMeas("find_good_features");
  // Find good features takes quite some runtime, only use it every x frames
  if (_framesSinceRefresh >= (_refreshAfter - 1)) {
    const int width = img.size().width;
    const int height = img.size().height;
    const int borderOutside = 50;
    cv::Mat mask = cv::Mat::zeros(height, width, CV_8U);
    cv::rectangle(mask, cv::Rect(borderOutside, borderOutside, width - (2*borderOutside), height - (2*borderOutside)), 255, cv::FILLED);
    cv::goodFeaturesToTrack(img, _prevFreatures, 200, 0.02, 7, mask);
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

  // _runtimeMeasService.printToConsole();
}
