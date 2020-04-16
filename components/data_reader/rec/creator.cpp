#include <fcntl.h>
#include <unistd.h>
#include <chrono>
#include "creator.h"
#include "../cams/rec_cam.h"


namespace data_reader {
  namespace rec {

    // Takes file of packed capnp frame data and creates the sensors accordingly
    void createFromFile(const std::string filePath, SensorStorage& storage, com_out::IRequestHandler& requestHandler) {
      const auto startTime = std::chrono::high_resolution_clock::now();

      int fd = open(filePath.c_str(), O_RDONLY);
      kj::FdInputStream fdStream(fd);
      kj::BufferedInputStreamWrapper bufferedStream(fdStream);
      std::vector<std::shared_ptr<RecCam>> recCams;
      std::map<std::string, RecCam::OwnCamFrames> framesPerCam;

      bool createCams = false;
      
      // Loop through complete thing
      while (bufferedStream.tryGetReadBuffer() != nullptr) {
        capnp::PackedMessageReader message(bufferedStream);
        auto frame = message.getRoot<CapnpOutput::Frame>();
        auto camSensors = frame.getCamSensors();
        for (int i = 0; i < camSensors.size(); ++i) {
          std::string key = camSensors[i].getKey();
          if (!createCams) {
            auto recCam = std::make_shared<RecCam>(
              key,
              camSensors[i].getFovHorizontal(),
              camSensors[i].getImg().getWidth(),
              camSensors[i].getImg().getHeight()
            );
            recCams.push_back(recCam);
            framesPerCam.insert({recCam->getName(), RecCam::OwnCamFrames()});
          }
          if (framesPerCam.find(key) != framesPerCam.end()) {
            auto capnpImg = std::make_shared<RecCam::OwnCamSensor>(newOwnCapnp(camSensors[i]));
            framesPerCam[key].push_back(capnpImg);
          }
        }
        createCams = true;
      }

      for (auto recCam: recCams) {
        requestHandler.registerRequestListener(recCam);
        recCam->setFrameData(framesPerCam[recCam->getName()]);
        storage.addCam(recCam, recCam->getName());
        recCam->start();
      }

      close(fd);

      const auto endTime = std::chrono::high_resolution_clock::now();
      const auto durAlgo = std::chrono::duration<double, std::milli>(endTime - startTime);
      std::cout << "Store Frames: " << durAlgo.count() << " [ms]" << std::endl;
    }
  }
}
