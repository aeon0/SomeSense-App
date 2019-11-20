#include "usb_cam.h"

UsbCam::UsbCam(int number) {
  _stream.open(number);
  if (!_stream.isOpened()) {
    throw std::runtime_error("UsbCam device could not be opened");
  }
}

cv::Mat UsbCam::getFrame() {
  cv::Mat frame;
  _stream.read(frame);
  return frame;
}
