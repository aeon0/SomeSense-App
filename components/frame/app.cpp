#include "app.h"
#include "com_out/unix_server.h"
#include <iostream>
#include <thread>


void frame::App::init(const std::string& sensorConfigPath) {
  _sensorStorage.initFromConfig(sensorConfigPath);
}

void frame::App::start() {
  std::thread serverThread(&com_out::Server::run, &_server);

  for(;;) {
    _server.broadcast("{\"type\": \"server.hello\", \"data\": \"hello world\"}\n");

    cv::namedWindow( "Display window", cv::WINDOW_AUTOSIZE );
    for(auto const& [key, cam]: _sensorStorage.getCams()) {
      cv::Mat img = cam->getFrame();
      // TODO: do the whole image processing stuff
      cv::imshow( "Display window", img );
      cv::waitKey(0);
    }
    // Loop through other sensor types if needed and do the processing

    // TODO: input all data to tracker

    // TODO: send created environment + image to the visualization
  }

  serverThread.join();
}
