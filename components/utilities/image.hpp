#pragma once
#include "opencv2/opencv.hpp"


namespace util {
  void cropAndResize(const cv::Mat& inputMat, cv::Mat& outputMat, int targetHeight, int targetWidth, int offsetBottom) {
    double currRatio = inputMat.size().width / static_cast<double>(inputMat.size().height);
    double targetRatio = targetWidth / static_cast<double>(targetHeight);
    int deltaHeight = inputMat.size().height - (inputMat.size().width / targetRatio);
    int deltaHeightTop = deltaHeight - offsetBottom;
    cv::Rect roi;
    roi.x = 0;
    roi.y = deltaHeightTop;
    roi.width = inputMat.size().width;
    roi.height = inputMat.size().height - deltaHeight;
    cv::Mat croppedImg = inputMat(roi);
    cv::resize(croppedImg, outputMat, cv::Size(targetWidth, targetHeight));
  }
}
