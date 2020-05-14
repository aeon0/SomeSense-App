#include "tracker.h"


tracking::Tracker::Tracker(frame::RuntimeMeasService& runtimeMeasService) :
  _runtimeMeasService(runtimeMeasService) {}

void tracking::Tracker::update() {
  
}

void tracking::Tracker::reset() {

}

void tracking::Tracker::serialize(CapnpOutput::Frame::Builder& builder) {
  // Some dummy tracks to test visu
  auto sts = builder.initTracks(2);
  sts[0].setTrackId(std::string("id1"));
  sts[0].setHeight(1.5);
  sts[0].setLength(4.5);
  sts[0].setWidth(2.5);
  sts[0].setX(38.0);
  sts[0].setY(3.0);
  sts[0].setZ(0.0);
  sts[0].setYaw(0);
  sts[0].setRoll(0);
  sts[0].setPitch(0);
  sts[0].setObjClass(0);
  sts[0].setVelocity(5.0);

  sts[0].setTrackId(std::string("id2"));
  sts[0].setHeight(1.5);
  sts[0].setLength(0.0); // no lenght -> 2d object
  sts[0].setWidth(2.5);
  sts[0].setX(20.0);
  sts[0].setY(-7.0);
  sts[0].setZ(0.0);
  sts[0].setYaw(0);
  sts[0].setRoll(0);
  sts[0].setPitch(0);
  sts[0].setObjClass(5);
  sts[0].setVelocity(5.0);
}
