#pragma once

#include <string>
#include <tuple>
#include <vector>
#include "utilities/json.hpp"

namespace output {

  struct CamSensor {
    int idx;
    std::string key;
    double position[3];
    double rotation[3];
    double fovHorizontal;
    double fovVertical;
  };
  // void to_json(nlohmann::json& j, const CamSensor& c) {
  //   j = nlohmann::json{
  //     {"idx", c.idx},
  //     {"key", c.key},
  //     {"position", c.position},
  //     {"rotation", c.rotation},
  //     {"fovHorizontal", c.fovHorizontal},
  //     {"fovVertical", c.fovVertical}
  //   };
  // }


  struct RuntimeMeas {
    std::string name;
    int64_t start;
    double duration;
  };
  // void to_json(nlohmann::json& j, const RuntimeMeas& r) {
  //   j = nlohmann::json{
  //     {"name", r.name},
  //     {"start", r.start},
  //     {"duration", r.duration}};
  // }


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
  // void to_json(nlohmann::json& j, const Track& t) {
  //   j = nlohmann::json{
  //     {"trackId", t.trackId},
  //     {"objClass", t.objClass},
  //     {"position", t.position},
  //     {"rotation", t.rotation},
  //     {"velocity", t.velocity},
  //     {"height", t.height},
  //     {"width", t.width},
  //     {"length", t.length},
  //     {"ttc", t.ttc}
  //   };
  // }

  struct OutputState {
    int64_t timestamp;
    int frame;
    std::vector<Track> tracks;

    // Debug data
    std::vector<CamSensor> sensors;
    std::vector<RuntimeMeas> runtimeMeas;
  };
  // void to_json(nlohmann::json& j, const OutputState& o) {
  //   j = nlohmann::json{
  //     {"timestamp", o.timestamp},
  //     {"frame", o.frame},
  //     {"tracks", o.tracks},
  //     {"sensors", o.sensors},
  //     {"runtimeMeas", o.runtimeMeas}
  //   };
  // }
}
