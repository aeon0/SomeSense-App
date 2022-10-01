#pragma once

#include "opencv2/opencv.hpp"
#include "img.h"
#include "camera.pb.h"


namespace util {

  template<typename T>
  void fillProtoImg(proto::Img* pbImg, const cv::Mat& cvImg, const img::Roi& roi) {
    pbImg->set_width(cvImg.size().width);
    pbImg->set_height(cvImg.size().height);
    pbImg->set_channels(cvImg.channels());
    pbImg->set_offsetleft(roi.offsetLeft);
    pbImg->set_offsettop(roi.offsetTop);
    pbImg->set_scale(roi.scale);
    pbImg->set_type(cvImg.type());
    pbImg->set_typestr(cv::typeToString(cvImg.type()));
    pbImg->set_data(cvImg.data, cvImg.size().width * cvImg.size().height * cvImg.channels() * sizeof(T));
  }

  void fillCvImg(cv::Mat& cvImg, const proto::Img& pbImg);
}
