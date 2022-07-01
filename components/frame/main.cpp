#include "frame/app.h"
#include "com_out/tcp_server.h"
#include "data_reader/sensor_storage.h"
#include "serialize/app_state.h"
#include "serialize/save_to_file.h"
#include <thread>
#include <iostream>


int main(int argc, char** argv) {
  // Create shared output storage memory
  auto appState = serialize::AppState();

  // Run Server
  com_out::TcpServer server(appState);
  std::thread serverThread(&com_out::Server::run, &server);
  std::thread serverOutputThread(&com_out::Server::pollOutput, &server);

  const auto algoStartTime = std::chrono::high_resolution_clock::now();

  // Create Sensor Storage
  assert(argc == 2 && "Missing argument for config path");
  const std::string sensorConfigPath = argv[1];
  auto sensorStorage = data_reader::SensorStorage(server, algoStartTime);
  sensorStorage.createFromConfig(sensorConfigPath, appState);

  // Create Storage Service for recording
  auto saveToFileService = std::make_shared<serialize::SaveToFile>("./storage_data/", appState);
  server.registerRequestListener(saveToFileService);

  // Start Algo Application
  std::cout << "** Start Application **" << std::endl;
  auto app = std::make_shared<frame::App>(sensorStorage, appState, algoStartTime);
  server.registerRequestListener(app);
  app->run();

  // Stop Server
  server.stop();
  serverOutputThread.detach();
  serverThread.detach();

  std::cout << std::endl << "** Exit Application **" << std::endl;
  return 0;
}
