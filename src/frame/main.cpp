#include <iostream>
#include <thread>
#include <atomic>
#include <queue>

#include "util/json.hpp"
#include "config.h"
#include "frame.pb.h"
#include "recmeta.pb.h"
#include "util/runtime_meas_service.h"
#include "util/time.h"
#include "data/sensor_storage.h"
#include "algo/scheduler.h"
#ifdef USE_ECAL // Set by CMake
#include "ecal/ecal_nodes.h"
#else

#endif


using namespace std::chrono_literals;

std::atomic<bool> play = false;
std::atomic<bool> playedOneFrame = false;
std::atomic<bool> doReset = false;
std::atomic<bool> syncLastFrame = false;
std::atomic<bool> stepForward = false;
std::atomic<bool> stepBack = false;
std::atomic<int64_t> jumpToRelTs = -1;
std::queue<proto::Frame> frameQueue;


void clientCallback(const std::string& request, std::string& response) {
  std::cout << std::endl;
  std::cout << "** Handle Client Request **" << std::endl;
  std::cout << request <<std::endl;
  response = "";
  try {
    auto jsonRequest = nlohmann::json::parse(request);
    
    if (jsonRequest.find("data") == jsonRequest.end()) {
      std::cout << "WARNING: data field not present in request, skipping" << std::endl;
      return;
    }

    std::string action = jsonRequest["data"].value("action", "unkown");
    if (action == "play") play = true;
    else if (action == "pause") play = false;
    else if (action == "reset") doReset = true;
    else if (action == "step_forward") stepForward = true;
    else if (action == "step_back") stepBack = true;
    else if (action == "jump_to_rel_ts") jumpToRelTs = jsonRequest["data"].value("rel_ts", -1);
    else if (action == "sync") syncLastFrame = true;
    else {
      std::cout << "WARNING: Unkown action " << action << std::endl;
    }

    nlohmann::json jsonResponse = {
      {"cbIndex", jsonRequest.value("cbIndex", -1)},
      {"data", ""},
    };
    response = jsonResponse.dump();
  }
  catch(nlohmann::detail::type_error e) {
    std::cout << "WARNING: Could not handle request" << std::endl;
  }
}

int main(int argc, char** argv) {
  std::cout << "** Start App **" << std::endl;

  // Init Coms
#ifdef USE_ECAL
  frame::initEcal();
  auto com = frame::EcalNodes();
#else

#endif

  // Creating Runtime Meas Service
  auto runtimeMeasService = util::RuntimeMeasService();

  // Create Sensor Storage
  assert(argc == 2 && "Missing argument for config path");
  auto sensorStorage = data::SensorStorage(runtimeMeasService);
  const std::string sensorConfigPath = argv[1];
  sensorStorage.createFromConfig(sensorConfigPath);
  auto const [isRec, recLength] = sensorStorage.getRecMeta();
  proto::RecMeta recMeta;
  recMeta.set_isrec(isRec);
  recMeta.set_reclength(recLength);

  // Creating algo instance
  auto scheduler = algo::Scheduler(runtimeMeasService);
  auto appStartTime = std::chrono::high_resolution_clock::now();

  while (com.isOk())
  {
    // Rec interactions
    // ============================================
    if (isRec) {
      if (!play && syncLastFrame) {
        syncLastFrame = false;
        if (frameQueue.size() > 0) {
          std::cout << "sync last frame" << std::endl;

          com.syncFrame(frameQueue.back());
          com.sendRecMeta(recMeta);
        }
      }
      if (doReset) {
        play = false;
        playedOneFrame = false;
        doReset = false;
        appStartTime = std::chrono::high_resolution_clock::now();
        sensorStorage.reset();
        scheduler.reset();
      }
      if (!play && playedOneFrame && !stepForward && !stepBack && jumpToRelTs == -1) {
        std::this_thread::sleep_for(1ms);
        // Clients should get updated in case recData stuff has changed
        if (play != recMeta.isplaying()) {
          recMeta.set_isplaying(play);
          com.sendRecMeta(recMeta);
        }
        std::this_thread::sleep_for(1ms);
        continue;
      }
      if ((stepBack || jumpToRelTs != -1) && frameQueue.size() > 0) {
        play = false;
        if (stepBack) {
          const auto lastRelTs = frameQueue.back().relts();
          const auto frameLength = frameQueue.back().plannedframelength() * 1000.0;
          jumpToRelTs = lastRelTs - frameLength;
        }
        sensorStorage.getRec()->setRelTs(jumpToRelTs);
        scheduler.reset();
      }
      // Reset any onetime actions
      playedOneFrame = true;
      stepForward = false;
      stepBack = false;
      jumpToRelTs = -1;
    }
    // ===============================================

    proto::Frame frame;
    // Take care of timestamps
    const auto absTs = std::chrono::high_resolution_clock::now();

    sensorStorage.fillFrame(frame, appStartTime);
    scheduler.exec(frame);

    runtimeMeasService.serialize(frame, absTs);
    runtimeMeasService.printToConsole();
    runtimeMeasService.reset();

    if (!isRec) {
      // Set timestamps properly, if rec all has been set within the fillFrame of sensorStorage
      frame.set_absts(util::timepointToInt64(absTs));
      frame.set_relts(util::calcDurationInInt64(absTs, appStartTime));
      frame.set_appstarttime(util::timepointToInt64(appStartTime));
      frame.set_plannedframelength(config::GOAL_FRAME_LENGTH);
      play = true; // For non recordings force play to true
    }
    recMeta.set_isplaying(play);

    com.sendFrame(frame);
    com.sendRecMeta(recMeta);

    frameQueue.push(frame);
    if (frameQueue.size() > 2) {
      frameQueue.pop();
    }

    // TODO: Better to have some interface on sensorStorage.getRec() for this?
    if (isRec && (frame.relts() + frame.plannedframelength() * 1000.0) > recMeta.reclength()) {
      play = false;
      std::cout << "** Pausing, reached end of recording **" << std::endl;
    }

    // Keep a consistent algo framerate
    auto goalFrameLen = isRec ? frame.plannedframelength() : config::GOAL_FRAME_LENGTH;
    const auto plannedFrameEnd = absTs + std::chrono::duration<double, std::milli>(goalFrameLen);
    std::this_thread::sleep_until(plannedFrameEnd);
  }

  com.finalize();
  std::cout << std::endl << "** Exit App **" << std::endl;
  return 0;
}
