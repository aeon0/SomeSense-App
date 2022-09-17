#include <ecal/ecal.h>
#include <ecal/msg/string/publisher.h>
#include <ecal/msg/capnproto/publisher.h>

#include <iostream>
#include <thread>

#include "config.h"
#include "interface/_gen/frame.capnp.h"
#include "util/runtime_meas_service.h"
#include "algo/scheduler/scheduler.h"


int main(int argc, char** argv) {
  std::cout << "** Start eCAL Node **" << std::endl;

  // Creating eCAL node
  eCAL::Initialize(argc, argv, "eCAL Node");
  eCAL::capnproto::CPublisher<CapnpOutput::Frame> publisher("ecal_node");

  // Creating Runtime Meas Service
  const auto algoStartTime = std::chrono::high_resolution_clock::now();
  auto runtimeMeasService = util::RuntimeMeasService(algoStartTime);

  // Create data reader
  int inputData = 0;
  int outputData;

  // Creating algo instance
  auto scheduler = algo::Scheduler(runtimeMeasService);

  while (eCAL::Ok())
  {
    const auto frameStart = std::chrono::high_resolution_clock::now();
    const auto plannedFrameEnd = frameStart + std::chrono::duration<double, std::milli>(config::GOAL_FRAME_LENGTH);

    int inputData = 0;
    int outputData;

    auto frameData = publisher.GetBuilder();
    scheduler.exec(inputData, outputData);
    publisher.Send();

    runtimeMeasService.printToConsole();
    runtimeMeasService.reset();

    frameData.setPlannedFrameLength(config::GOAL_FRAME_LENGTH);

    // Keep a consistent algo framerate
    // TODO: What to do if we want to speed up things?
    std::this_thread::sleep_until(plannedFrameEnd);
  }

  eCAL::Finalize();
  std::cout << std::endl << "** Exit eCAL Node **" << std::endl;
  return 0;
}
