#include <ecal/ecal.h>
#include <ecal/msg/string/publisher.h>
#include <ecal/msg/protobuf/publisher.h>

#include <iostream>
#include <thread>

#include "config.h"
#include "frame.pb.h"
#include "util/runtime_meas_service.h"
#include "algo/scheduler/scheduler.h"


int main(int argc, char** argv) {
  std::cout << "** Start eCAL Node **" << std::endl;

  // Creating eCAL node
  eCAL::Initialize(argc, argv, "eCAL Node");
  eCAL::protobuf::CPublisher<proto::Frame> publisher("somesense_app");

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

    proto::Frame data;
    data.set_timestamp(100);
    // data.mutable_camsensors()->Add();
    publisher.Send(data);
    scheduler.exec(inputData, outputData);

    runtimeMeasService.printToConsole();
    runtimeMeasService.reset();

    // Keep a consistent algo framerate
    // TODO: What to do if we want to speed up things?
    std::this_thread::sleep_until(plannedFrameEnd);
  }

  eCAL::Finalize();
  std::cout << std::endl << "** Exit eCAL Node **" << std::endl;
  return 0;
}
