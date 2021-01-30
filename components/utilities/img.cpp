#include "img.h"


util::img::Roi util::img::cropAndResize(const cv::Mat& inputMat, cv::Mat& outputMat, int targetHeight, int targetWidth, int offsetBottom) {
  // First lets cut the offsetBottom
  cv::Rect cutOffsetBottom;
  cutOffsetBottom.x = 0;
  cutOffsetBottom.y = 0;
  cutOffsetBottom.width = inputMat.size().width;
  cutOffsetBottom.height = inputMat.size().height - offsetBottom;
  cv::Mat offsetInputMat = inputMat(cutOffsetBottom);

  // Adjust to aspect ratio by either removing from left/right or removing from top of image
  double currRatio = offsetInputMat.size().width / static_cast<double>(offsetInputMat.size().height);
  double targetRatio = targetWidth / static_cast<double>(targetHeight);
  cv::Rect cutOut;
  Roi roi;
  roi.offsetLeft = 0.0;
  roi.offsetTop = 0.0;
  roi.scale = 1.0;
  if (currRatio > targetRatio) {
    const int deltaWidth = -int((targetRatio * offsetInputMat.size().height) - offsetInputMat.size().width);
    roi.offsetLeft = int(deltaWidth * 0.5);
    cutOut.x = roi.offsetLeft;
    cutOut.y = 0;
    cutOut.width = offsetInputMat.size().width - deltaWidth;
    cutOut.height = offsetInputMat.size().height;
    roi.scale = targetHeight / float(offsetInputMat.size().height);
  }
  else {
    roi.offsetTop = offsetInputMat.size().height - (offsetInputMat.size().width / targetRatio);
    cutOut.x = 0;
    cutOut.y = roi.offsetTop;
    cutOut.width = offsetInputMat.size().width;
    cutOut.height = offsetInputMat.size().height - roi.offsetTop;
    roi.scale = targetWidth / float(inputMat.size().width);
  }
  cv::Mat croppedImg = offsetInputMat(cutOut);
  cv::resize(croppedImg, outputMat, cv::Size(targetWidth, targetHeight));

  return roi;
}

cv::Point2f util::img::convertToRoi(const Roi& roi, const cv::Point2f& point) {
  cv::Point2f converted;
  converted.x = (1 / roi.scale) * point.x;
  converted.y = (1 / roi.scale) * point.y;
  converted.x += roi.offsetLeft;
  converted.y += roi.offsetTop;
  return converted;
}
