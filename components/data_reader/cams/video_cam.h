#ifndef video_cam_h
#define video_cam_h

#include "icam.h"

namespace data_reader {
  class VideoCam : public ICam {
  public:
    VideoCam(std::string filename);
    cv::Mat getFrame();

  private:
    cv::VideoCapture _stream;
    std::string _filename;
  };
}

#endif
