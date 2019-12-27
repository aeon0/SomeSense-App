#include "app.h"
#include "com_out/unix_server.h"
#include <thread>
#include <iostream>


int main() {
  // Create Server in seperate thread
  com_out::UnixServer unixServer;
  std::thread serverThread(&com_out::Server::run, &unixServer);

  const std::string sensorConfigPath = "configs/live_sensors.json";

  std::cout << "** Start Application **" << std::endl;
  auto app = std::make_shared<frame::App>(sensorConfigPath);
  unixServer.registerRequestListener(app);
  app->run(unixServer);
  unixServer.deleteRequestListener(app);

  serverThread.detach(); // Detach will terminate the server which is in accept mode

  std::cout << std::endl << "** Exit Program **" << std::endl;
  return 0;
}
