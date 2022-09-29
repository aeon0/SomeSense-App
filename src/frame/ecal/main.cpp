#include <ecal/ecal.h>
#include <ecal/msg/string/publisher.h>
#include <ecal/msg/protobuf/publisher.h>
#include <ecal/ecal_server.h>

#include <iostream>
#include <thread>
#include <atomic>
#include <queue>

#include "util/json.hpp"
#include "config.h"
#include "frame.pb.h"
#include "util/runtime_meas_service.h"
#include "data/sensor_storage.h"
#include "algo/scheduler/scheduler.h"


using namespace std::chrono_literals;

std::atomic<bool> play = true;
std::atomic<bool> playedOneFrame = false;
std::atomic<bool> doReset = false;
std::atomic<bool> sendLastFrame = false;
std::queue<proto::Frame> frameQueue;


int methodCallback(const std::string& method, const std::string& request, std::string& response) {
  std::cout << "Got request for method: " << method << std::endl;
  if (method == config::SERVER_METHOD_FRAME_CTRL) {
    auto jsonRequest = nlohmann::json::parse(request);

    std::string action = jsonRequest["action"];
    if (action == "play") play = true;
    else if (action == "pause") play = false;
    else if (action == "reset") doReset = true;
    else {
      std::cout << "WARNING: Unkown action " << action << std::endl;
    }

    nlohmann::json jsonResponse = {
      {"cbIndex", jsonRequest.value("cbIndex", -1)},
    };
  }
  else {
    std::cout << "WARNING: Unkown method request " << method << std::endl;
  }
  return 0;
}

void newClient(const char* name, const struct eCAL::SServerEventCallbackData* data) {
  std::cout << "New Client connected" << std::endl;
  sendLastFrame = true;
}

int main(int argc, char** argv) {
  std::cout << "** Start eCAL Node **" << std::endl;

  // Creating eCAL node
  eCAL::Initialize(argc, argv, "eCAL Node");
  eCAL::protobuf::CPublisher<proto::Frame> publisher(config::PUBLISHER_NAME);
  eCAL::CServiceServer server(config::SERVER_SERVICE_NAME);
  using namespace std::placeholders;
  server.AddMethodCallback(config::SERVER_METHOD_FRAME_CTRL, "", "", std::bind(methodCallback, _1, _4, _5));
  server.AddEventCallback(eCAL_Server_Event::server_event_connected, std::bind(newClient, _1, _2));

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
    if (sendLastFrame) {
      sendLastFrame = false;
      if (frameQueue.size() > 0) {
        publisher.Send(frameQueue.back());
      }
    }

    if (doReset) {
      play = false;
      playedOneFrame = false;
      doReset = false;
      sensorStorage.reset();
      scheduler.reset();
    }

    if (!play && playedOneFrame) {
      std::this_thread::sleep_for(1ms);
      continue;
    }
    proto::Frame frame;

    // Take care of timestamps
    const auto frameStart = std::chrono::high_resolution_clock::now();
    const auto plannedFrameEnd = frameStart + std::chrono::duration<double, std::milli>(config::GOAL_FRAME_LENGTH);
    const auto ts = static_cast<int64_t>(std::chrono::duration<double, std::micro>(frameStart - appStartTime).count());
    frame.set_timestamp(ts);
    // TODO: Convert this properly to int64_t
    // frame.set_appstarttime(appStartTime);
    frame.set_plannedframelength(config::GOAL_FRAME_LENGTH);

    sensorStorage.fillFrame(frame);
    // Fill rec data (isRec and recLength is filled by sensorStorage)
    frame.mutable_recdata()->set_isplaying(true);

    scheduler.exec(frame);

    runtimeMeasService.serialize(frame);
    runtimeMeasService.printToConsole();
    runtimeMeasService.reset();

    playedOneFrame = true;
    publisher.Send(frame);
    frameQueue.push(frame);
    if (frameQueue.size() > 2) {
      frameQueue.pop();
    }

    // Keep a consistent algo framerate
    // TODO: What to do if we want to speed up things?
    std::this_thread::sleep_until(plannedFrameEnd);
  }

  eCAL::Finalize();
  std::cout << std::endl << "** Exit eCAL Node **" << std::endl;
  return 0;
}
