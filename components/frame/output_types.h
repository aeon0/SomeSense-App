#pragma once

#include <string>
#include <tuple>
#include <vector>


namespace output {
  struct CamSensor {
    int idx;
    std::string key;
    double position[3];
    double rotation[3];
    double fovHorizontal;
    double fovVertical;
  };

  struct RuntimeMeas {
    std::string name;
    int64_t start;
    double duration;
  };

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

  struct OutputState {
    int64_t timestamp;
    int frame;
    std::vector<Track> tracks;

    // Debug data
    std::vector<CamSensor> sensors;
    std::vector<RuntimeMeas> runtimeMeas;
  };
}
