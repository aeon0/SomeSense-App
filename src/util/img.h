#pragma once
#include "opencv2/opencv.hpp"


namespace util {
  namespace img {
    struct Roi {
      int offsetTop;
      int offsetLeft;
      float scale;
    };

    Roi cropAndResize(const cv::Mat& inputMat, cv::Mat& outputMat, int targetHeight, int targetWidth, int offsetBottom);

    cv::Point2f convertToRoi(const Roi& roi, const cv::Point2f& point);

    cv::Point2f convertToRoiInv(const Roi& roi, const cv::Point2f& point);

    cv::Point3f unit(cv::Point3f vec);

    cv::Point2f unit(cv::Point2f vec);
  }
}
