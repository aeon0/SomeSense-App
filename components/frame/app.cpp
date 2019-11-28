#include "app.h"
#include "com_out/unix_server.h"
#include <iostream>


void frame::App::init(const std::string& sensorConfigPath) {
  _sensorStorage.initFromConfig(sensorConfigPath);
}

void frame::App::start() {
  _server.run();

  for(;;) {
    cv::namedWindow( "Display window", cv::WINDOW_AUTOSIZE );
    std::cout << "Run Frame" << std::endl;
    for(auto const& [key, cam]: _sensorStorage.getCams()) {
      cv::Mat img = cam->getFrame();
      // TODO: do the whole image processing stuff
      cv::imshow( "Display window", img );
      cv::waitKey(0);
    }
    // Loop through other sensor types if needed and do the processing

    // TODO: start the fusion to start the object tracking

    // TODO: send created environment + image to the visualization
  }
}
