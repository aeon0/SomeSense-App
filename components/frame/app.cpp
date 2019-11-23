#include "app.h"
#include <iostream>

void frame::App::init(const std::string& sensorConfigPath) {
  _sensorStorage.initFromConfig(sensorConfigPath);
}

void frame::App::start() {
  for(auto const& [key, cam]: _sensorStorage.getCams()) {
    std::cout << "Getting frame" << std::endl;
    cv::Mat img = cam->getFrame();
    cv::namedWindow( "Display window", cv::WINDOW_AUTOSIZE );// Create a window for display.
    cv::imshow( "Display window", img );                   // Show our image inside it.

    cv::waitKey(0);                                          // Wait for a keystroke in the window
  }

  //for(;;) {
  //  std::cout << "Run Frame" << std::endl;
  //}
}
