#include "app.h"
#include "com_out/tcp_server.h"
#include "data_reader/sensor_storage.h"
#include "output/storage.h"
#include "output/storage_service.h"
#include <thread>
#include <iostream>


int main() {
  // Create shared output storage memory
  auto outputStorage = output::Storage();

  // Run Server
  com_out::TcpServer server(outputStorage);
  std::thread serverThread(&com_out::Server::run, &server);
  std::thread serverOutputThread(&com_out::Server::pollOutput, &server);

  const auto algoStartTime = std::chrono::high_resolution_clock::now();

  // Create Sensor Storage
  const std::string sensorConfigPath = "configs/sim_sensors.json";
  auto sensorStorage = data_reader::SensorStorage(server, algoStartTime, outputStorage);
  sensorStorage.initFromConfig(sensorConfigPath);

  // Create Storage Service for recording
  auto storageService = std::make_shared<output::StorageService>("./storage_data/", outputStorage);
  server.registerRequestListener(storageService);

  // Start Algo Application
  std::cout << "** Start Application **" << std::endl;
  auto app = std::make_shared<frame::App>(sensorStorage, outputStorage, algoStartTime);
  app->run();

  // Stop Server
  server.stop();
  serverOutputThread.detach();
  serverThread.detach();

  std::cout << std::endl << "** Exit Application **" << std::endl;
  return 0;
}
