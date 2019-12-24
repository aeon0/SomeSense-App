#include "app.h"
#include "com_out/unix_server.h"
#include <thread>
#include <iostream>

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>


// TODO: should be sig_atomic_t, but then it can not be passed to app.run() and
//       I am not sure how to stop the app running otherwise...
// volatile sig_atomic_t stop = 0;
int stop = 0;
void sighandler(int signum) {
  stop = 1;
}

int main() {
  // Create Server in seperate thread
  com_out::UnixServer unixServer;
  std::thread serverThread(&com_out::Server::run, &unixServer);

  // Listen to SIGINT (usually ctrl + c on terminal), has to be after the server thread!
  signal(SIGINT, &sighandler);

  const std::string sensorConfigPath = "configs/sim_sensors.json";

  // This structure gives the app the possibilitiy to "restart"
  // by just exiting out of the loop used within start() and reinitialize its objects
  while (!stop) {
    std::cout << "** Start Application **" << std::endl;
    frame::App app;
    app.init(sensorConfigPath);
    app.run(unixServer, stop);
  }

  serverThread.detach(); // Detach will terminate the server which is in accept mode

  std::cout << "** Exit Program **" << std::endl;
  return 0;
}
