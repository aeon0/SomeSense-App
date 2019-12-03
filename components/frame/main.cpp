#include "app.h"
#include "object_detection/detector.h"


int main() {
  // Inputs
  /*
  const std::string sensorConfigPath = "configs/sim_sensors.json";

  std::unique_ptr<frame::App> app(new frame::App());
  app->init(sensorConfigPath);
  app->start();
  */
  object_detection::Detector detector;
  detector.test();

  return 0;
}
