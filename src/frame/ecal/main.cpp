#include <ecal/ecal.h>
#include <ecal/msg/string/publisher.h>
#include <ecal/msg/capnproto/publisher.h>

#include <iostream>
#include <thread>

#include "interface/_gen/frame.capnp.h"
#include "algo/scheduler/scheduler.h"


int main(int argc, char** argv) {
  std::cout << "** Start eCAL Node **" << std::endl;

  // Creating algo instance
  const auto algoStartTime = std::chrono::high_resolution_clock::now();
  auto scheduler = algo::Scheduler(algoStartTime);

  // Creating data reader instance

  // Creating eCAL node
  eCAL::Initialize(argc, argv, "eCAL Node");
  eCAL::capnproto::CPublisher<CapnpOutput::Frame> publisher("ecal_node");

  // Create a counter, so something changes in our message
  int inputData = 0;
  int outputData;

  while (eCAL::Ok())
  {
    auto frameData = publisher.GetBuilder();
    scheduler.exec(inputData, outputData);
    publisher.Send();

    // Sleep 200 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  eCAL::Finalize();
  std::cout << std::endl << "** Exit eCAL Node **" << std::endl;
  return 0;
}
