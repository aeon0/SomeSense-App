#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "data_reader/sensor_storage.h"
#include "serialize/app_state.h"
#include "frame/app.h"


class DummyRequestHandler : public com_out::IRequestHandler {
  public:
    void registerRequestListener(std::shared_ptr<com_out::IRequestListener> listener) {};
    void deleteRequestListener(std::shared_ptr<com_out::IRequestListener> listener) {};
};

TEST(Frame, runFrame)
{
  // Create shared output storage memory
  auto appState = serialize::AppState();
  const auto algoStartTime = std::chrono::high_resolution_clock::now();

  auto requestHandler = DummyRequestHandler();
  auto sensorStorage = data_reader::SensorStorage(requestHandler, algoStartTime);

  // // Start Algo Application
  auto app = std::make_shared<frame::App>(sensorStorage, appState, algoStartTime);
  app->runFrame();

  // Do some testing
}
