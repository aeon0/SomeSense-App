#include "types.h"


void output::to_json(nlohmann::json& j, const CamSensor& c) {
  j = nlohmann::json {
    {"idx", c.idx},
    {"key", c.key},
    {"position", c.position},
    {"rotation", c.rotation},
    {"fovHorizontal", c.fovHorizontal},
    {"fovVertical", c.fovVertical}
  };
}

void output::to_json(nlohmann::json& j, const RuntimeMeas& r) {
  j = nlohmann::json {
    {"name", r.name},
    {"start", r.start},
    {"duration", r.duration}};
}

void output::to_json(nlohmann::json& j, const Track& t) {
  j = nlohmann::json {
    {"trackId", t.trackId},
    {"objClass", t.objClass},
    {"position", t.position},
    {"rotation", t.rotation},
    {"velocity", t.velocity},
    {"height", t.height},
    {"width", t.width},
    {"length", t.length},
    {"ttc", t.ttc}
  };
}

void output::to_json(nlohmann::json& j, const Frame& f) {
  j = nlohmann::json {
    {"timestamp", f.timestamp},
    {"frameCount", f.frameCount},
    {"tracks", f.tracks},
    {"camSensors", f.camSensors},
    {"runtimeMeas", f.runtimeMeas}
  };
}

void output::to_json(nlohmann::json& j, const ControlData& c) {
  j = nlohmann::json {
    {"isStoring", c.isStoring},
    {"isARecording", c.isARecording},
    {"isPlaying", c.isPlaying},
    {"recLength", c.recLength}
  };
}
