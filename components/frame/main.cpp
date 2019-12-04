#include "app.h"


int main() {
  // Inputs
  const std::string sensorConfigPath = "configs/sim_sensors.json";

  std::unique_ptr<frame::App> app(new frame::App());
  app->init(sensorConfigPath);
  app->start();

  return 0;
}
