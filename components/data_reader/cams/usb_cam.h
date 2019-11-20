#ifndef UsbCam_H
#define UsbCam_H

#include "icam.h"


class UsbCam : public ICam {
public:
  UsbCam(int number = 0);
  cv::Mat getFrame();

private:
  cv::VideoCapture _stream;
};

#endif
