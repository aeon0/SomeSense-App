#include "csi_cam.h"
#include "util/proto.h"


std::string gstreamer_pipeline(int captureWidth, int captureHeight, int frameRate, int flipMethod) {
  return "nvarguscamerasrc ! video/x-raw(memory:NVMM), width=(int)" + std::to_string(captureWidth) + ", height=(int)" +
          std::to_string(captureHeight) + ", format=(string)NV12, framerate=(fraction)" + std::to_string(frameRate) +
          "/1 ! nvvidconv flip-method=" + std::to_string(flipMethod) + " ! videoconvert ! video/x-raw, format=(string)BGR ! appsink";
}

data::CsiCam::CsiCam(
  std::string camName,
  int captureWidth,
  int captureHeight,
  int horizontalFov,
  double frameRate,
  int flipMethod
) :
  _name(camName)
{
  std::cout << "** Creating CSI camera **" << std::endl;
  std::string gsStreamerPipline = gstreamer_pipeline(captureWidth, captureHeight, frameRate, flipMethod);
  std::cout << "Using GStream Pipline: " << gsStreamerPipline << std::endl;

  _capture.open(gsStreamerPipline, cv::CAP_GSTREAMER);
  if (!_capture.isOpened()) {
    throw std::runtime_error("Could not open CSI Camera: " + _name);
  }

  _cam = util::Cam();
  _cam.setIntrinsics(_capture.get(cv::CAP_PROP_FRAME_WIDTH), _capture.get(cv::CAP_PROP_FRAME_HEIGHT), horizontalFov);

  _roi.scale = 1.0;
  _roi.offsetLeft = 0.0;
  _roi.offsetTop = 0.0;
}

void data::CsiCam::fillCamData(proto::CamSensor& camSensor, const util::TS& appStartTime) {
  const auto captureTime = std::chrono::high_resolution_clock::now();
  bool success = _capture.read(_currFrame);
  if (success) {
    camSensor.set_absts(util::timepointToInt64(captureTime));
    camSensor.set_relts(util::calcDurationInInt64(captureTime, appStartTime));

    auto img = camSensor.mutable_img();
    util::fillProtoImg<uchar>(img, _currFrame, _roi);

    // set intrinsics
    _cam.fillProtoCalib(camSensor.mutable_calib());
  }
  camSensor.set_isvalid(success);
}
