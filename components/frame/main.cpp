#include "app.h"
#include "com_out/tcp_server.h"
#include "data_reader/sensor_storage.h"
#include "output/storage.h"
#include <thread>
#include <iostream>


int main() {
  // Create Server in seperate thread
  com_out::TcpServer server;
  std::thread serverThread(&com_out::Server::run, &server);

  const auto algoStartTime = std::chrono::high_resolution_clock::now();

  auto outputStorage = output::Storage();

  const std::string sensorConfigPath = "configs/live_sensors_usb.json";
  auto sensorStorage = data_reader::SensorStorage(algoStartTime);
  sensorStorage.initFromConfig(sensorConfigPath);

  std::cout << "** Start Application **" << std::endl;
  auto app = std::make_shared<frame::App>(sensorStorage, outputStorage, algoStartTime);
  app->run(server);

  // TODO: app should either store its output and server reads it in its thread
  //       or app should get a shared memory to write to where server sends it every x times

  serverThread.detach(); // Detach will terminate the server which is in accept mode

  std::cout << std::endl << "** Exit Program **" << std::endl;
  return 0;
}
