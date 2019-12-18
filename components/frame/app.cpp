#include "app.h"
#include "com_out/unix_server.h"
#include <iostream>
#include <thread>
#include <cmath>
#include "utilities/base64.h"
#include "utilities/json.hpp"


void frame::App::init(const std::string& sensorConfigPath) {
  _sensorStorage.initFromConfig(sensorConfigPath);
  _detector.loadModel("assets/od_model/model.onnx", "assets/od_model/prior_boxes.json");
}

void frame::App::start() {
  std::thread serverThread(&com_out::Server::run, &_server);

  for(;;) {
    cv::namedWindow( "Display window", cv::WINDOW_AUTOSIZE );
    for(auto const& [key, cam]: _sensorStorage.getCams()) {
      cv::Mat img = cam->getFrame();
      // TODO: do the whole image processing stuff
      _detector.detect(img);

      // Some test data to send
      std::vector<uchar> buf;
      cv::imencode(".jpg", img, buf);
      auto *encMsg = reinterpret_cast<unsigned char*>(buf.data());
      std::string encodedBase64Img = base64_encode(encMsg, buf.size());
      encodedBase64Img = "data:image/jpeg;base64," + encodedBase64Img;

      const float fovHorizontal = M_PI * 0.33f;
      const float fovVertical = M_PI * 0.25f;

      nlohmann::json j = {
        {"type", "server.frame"},
        {"data", {
          {"tracks", {{
            {"trackId", "0"},
            {"class", 0},
            {"position", {5.0, 0.0, 10.0}},
            {"rotation", {0.0, 0.0, 0.0}},
            {"height", 1.5},
            {"width", 0.5},
            {"depth", 3.0},
            {"ttc", 1.0}
          }}},
          {"sensor", {
            {"position", {0, 1.2, -0.5}},
            {"rotation", {0, 0, 0}},
            {"fovHorizontal", fovHorizontal},
            {"fovVertical", fovVertical},
            {"imageBase64", encodedBase64Img}
          }}
        }}
      };

      std::string outputState = j.dump();
      _server.broadcast(outputState + "\n");

      cv::imshow( "Display window", img );
      cv::waitKey(0);
    }
    // Loop through other sensor types if needed and do the processing

    // TODO: input all data to tracker

    // TODO: send created environment + image to the visualization
  }

  serverThread.join();
}
