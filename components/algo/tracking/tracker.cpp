#include "tracker.h"
#include "utilities/img.h"


tracking::Tracker::Tracker(frame::RuntimeMeasService& runtimeMeasService) :
  _runtimeMeasService(runtimeMeasService) {}

void tracking::Tracker::reset() {
  _tracks.clear();
}

void tracking::Tracker::update(const data_reader::ICam& cam) {
  _tracks.clear();
}

void tracking::Tracker::serialize(CapnpOutput::Frame::Builder& builder) {
  auto tracks = builder.initTracks(_tracks.size());
  for (int i = 0; i < _tracks.size(); ++i) {
    tracks[i].setTrackId(std::string("id" + std::to_string(i)));
    tracks[i].setHeight(1.0);
    tracks[i].setLength(1.0);
    tracks[i].setWidth(1.0);
    tracks[i].setX(_tracks[i].coord.x);
    tracks[i].setY(_tracks[i].coord.y);
    tracks[i].setZ(0.5);
    tracks[i].setYaw(0);
    tracks[i].setRoll(0);
    tracks[i].setPitch(0);
    tracks[i].setObjClass(0);
    tracks[i].setVelocity(0.0);
  }
}
