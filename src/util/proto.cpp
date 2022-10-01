#include "proto.h"


void util::fillCvImg(cv::Mat& cvImg, const proto::Img& pbImg) {
  int height = pbImg.height();
  int width = pbImg.width();
  int channels = pbImg.channels();
  void* dataPtr = const_cast<char*>(pbImg.data().c_str());
  cvImg = cv::Mat(height, width, pbImg.type(), dataPtr);
}
