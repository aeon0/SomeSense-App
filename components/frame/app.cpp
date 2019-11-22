#include <fstream>
#include "app.h"
#include "utilities/json.hpp"

void frame::App::init() {
  // add cameras
  std::string sensorConfigPath = "configs/sim_sensors.json";
  std::ifstream ifs(sensorConfigPath);
  if (!ifs.good()) {
    throw std::runtime_error("Could not open sensor config file:" + sensorConfigPath );
  }
  nlohmann::json sensorConfig = nlohmann::json::parse(ifs);
  // Loop over sensorConfig["cams"]
  std::cout << sensorConfig["cams"] << std::endl;
}

void frame::App::start() {
  for(;;) {
    std::cout << "Run Frame" << std::endl;
  }
}
