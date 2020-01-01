#include "app.h"
#include "com_out/tcp_server.h"
#include <thread>
#include <iostream>


int main() {
  // Create Server in seperate thread
  com_out::TcpServer server;
  std::thread serverThread(&com_out::Server::run, &server);

  const std::string sensorConfigPath = "configs/live_sensors_csi.json";

  std::cout << "** Start Application **" << std::endl;
  auto app = std::make_shared<frame::App>(sensorConfigPath);
  server.registerRequestListener(app);
  app->run(server);
  server.deleteRequestListener(app);

  serverThread.detach(); // Detach will terminate the server which is in accept mode

  std::cout << std::endl << "** Exit Program **" << std::endl;
  return 0;
}
