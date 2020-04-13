#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "data_reader/sensor_storage.h"
#include "output/storage.h"
#include "frame/app.h"


class DummyRequestHandler : public com_out::IRequestHandler {
  public:
    void registerRequestListener(std::shared_ptr<com_out::IRequestListener> listener) {};
    void deleteRequestListener(std::shared_ptr<com_out::IRequestListener> listener) {};
};

TEST(Frame, runFrame)
{
  // Create shared output storage memory
  auto outputStorage = output::Storage();
  const auto algoStartTime = std::chrono::high_resolution_clock::now();

  auto requestHandler = DummyRequestHandler();
  auto sensorStorage = data_reader::SensorStorage(requestHandler, algoStartTime, outputStorage);

  // // Start Algo Application
  auto app = std::make_shared<frame::App>(sensorStorage, outputStorage, algoStartTime);
  app->runFrame();

  // Do some testing
}
