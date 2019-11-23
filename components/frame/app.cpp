#include "app.h"

void frame::App::init(const std::string& sensorConfigPath) {
  _sensorStorage.initFromConfig(sensorConfigPath);
}

void frame::App::start() {
  data_reader::SensorStorage::CamMap cams = _sensorStorage.getCams();
  for( auto const& [key, cam]: cams) {
    //cv::Mat img = cam.getFrame();
  }

  //for(;;) {
  //  std::cout << "Run Frame" << std::endl;
  //}
}
