#pragma once
#include "opencv2/opencv.hpp"


namespace util {
  struct Roi {
    int offsetTop;
    int offsetLeft;
    float scale;
  };

  Roi cropAndResize(const cv::Mat& inputMat, cv::Mat& outputMat, int targetHeight, int targetWidth, int offsetBottom) {
    double currRatio = inputMat.size().width / static_cast<double>(inputMat.size().height);
    double targetRatio = targetWidth / static_cast<double>(targetHeight);
    int deltaHeight = inputMat.size().height - (inputMat.size().width / targetRatio);
    int deltaHeightTop = deltaHeight - offsetBottom;
    cv::Rect cutOut;
    cutOut.x = 0;
    cutOut.y = deltaHeightTop;
    cutOut.width = inputMat.size().width;
    cutOut.height = inputMat.size().height - deltaHeight;
    cv::Mat croppedImg = inputMat(cutOut);
    cv::resize(croppedImg, outputMat, cv::Size(targetWidth, targetHeight));

    Roi roi;
    roi.offsetLeft = 0;
    roi.offsetTop = deltaHeightTop;
    roi.scale = targetWidth / float(inputMat.size().width);

    return roi;
  }

  cv::Point2f convertToRoi(const Roi& roi, const cv::Point2f& point)
  {
    cv::Point2f converted;
    converted.x = (1 / roi.scale) * point.x;
    converted.y = (1 / roi.scale) * point.y;
    converted.x += roi.offsetLeft;
    converted.y += roi.offsetTop;
    return converted;
  }
}
