#include <ecal/ecal.h>
#include <ecal/msg/string/publisher.h>
#include <ecal/msg/protobuf/publisher.h>
#include <ecal/ecal_server.h>

#include <iostream>
#include <thread>

#include "config.h"
#include "frame.pb.h"
#include "util/runtime_meas_service.h"
#include "data/sensor_storage.h"
#include "algo/scheduler/scheduler.h"


int main(int argc, char** argv) {
  std::cout << "** Start eCAL Node **" << std::endl;

  // Creating eCAL node
  eCAL::Initialize(argc, argv, "eCAL Node");
  eCAL::protobuf::CPublisher<proto::Frame> publisher("somesense_app");
  eCAL::CServiceServer server("somesense_server");

  // Creating Runtime Meas Service
  const auto appStartTime = std::chrono::high_resolution_clock::now();
  auto runtimeMeasService = util::RuntimeMeasService(appStartTime);

  // Create data reader
  int inputData = 0;
  int outputData;

  // Create Sensor Storage
  assert(argc == 2 && "Missing argument for config path");
  const std::string sensorConfigPath = argv[1];
  auto sensorStorage = data::SensorStorage(runtimeMeasService);
  sensorStorage.createFromConfig(sensorConfigPath);

  // Creating algo instance
  auto scheduler = algo::Scheduler(runtimeMeasService);

  while (eCAL::Ok())
  {
    proto::Frame frame;

    // Take care of timestamps
    const auto frameStart = std::chrono::high_resolution_clock::now();
    const auto plannedFrameEnd = frameStart + std::chrono::duration<double, std::milli>(config::GOAL_FRAME_LENGTH);
    const auto ts = static_cast<int64_t>(std::chrono::duration<double, std::micro>(frameStart - appStartTime).count());
    frame.set_timestamp(ts);
    frame.set_plannedframelength(config::GOAL_FRAME_LENGTH);

    sensorStorage.fillFrame(frame);
    scheduler.exec(frame);

    runtimeMeasService.serialize(frame);
    runtimeMeasService.printToConsole();
    runtimeMeasService.reset();

    publisher.Send(frame);

    // Keep a consistent algo framerate
    // TODO: What to do if we want to speed up things?
    std::this_thread::sleep_until(plannedFrameEnd);
  }

  eCAL::Finalize();
  std::cout << std::endl << "** Exit eCAL Node **" << std::endl;
  return 0;
}
