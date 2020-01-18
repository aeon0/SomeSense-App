#pragma once

#include <string>
#include <tuple>
#include <vector>
#include <opencv2/core/core.hpp>
#include "utilities/json.hpp"

#define OUTPUT_INTERFACE_VERSION 1


namespace output {

  struct CamSensor {
    int idx;
    std::string key;
    double position[3];
    double rotation[3];
    double fovHorizontal;
    double fovVertical;
  };
  void to_json(nlohmann::json& j, const CamSensor& c);

  struct RuntimeMeas {
    std::string name;
    int64_t start;
    double duration;
  };
  void to_json(nlohmann::json& j, const RuntimeMeas& r);

  struct Track {
    std::string trackId;
    int objClass;
    double position[3];
    double rotation[3];
    double velocity;
    double height;
    double width;
    double length;
    double ttc;
  };
  void to_json(nlohmann::json& j, const Track& t);

  struct Frame {
    int64_t timestamp;
    int frameCount;
    std::vector<Track> tracks;

    // Debug data, maybe create seperate struct for this
    std::vector<CamSensor> camSensors;
    std::vector<RuntimeMeas> runtimeMeas;
  };
  void to_json(nlohmann::json& j, const Frame& o);

  struct CamImg {
    int sensorIdx;
    int64_t timestamp;
    cv::Mat img;
    int width;
    int height;
    int channels;
  };
}
