#include "rec_cam.h"
#include <fstream>
#include <string>
#include <unordered_map>
#include <iostream>
#include <mutex>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <thread>
#include <capnp/serialize.h>
#include "utilities/json.hpp"


data_reader::RecCam::RecCam(
  const std::string name,
  const double horizontalFov,
  const int width,
  const int height,
  serialize::AppState& appState,
  std::string filePath
) : 
  BaseCam(name, std::chrono::high_resolution_clock::now()),
  _gotOneFrame(false),
  _pause(true),
  _jumpToFrame(false),
  _currFrameNr(0),
  _appState(appState),
  _filePath(filePath)
{
  setIntrinsics(width, height, horizontalFov);
  setExtrinsics(0.0, 0.0, 1.7, 0.0, 0.0, 0.0);

  // Try to read the msg timestamps form a tmp json file in case it was previously loaded
  auto filePathHash = std::to_string(std::hash<std::string>{}(filePath));
  auto msgStartFilePath = "./tmp/" + filePathHash + ".json";
  std::ifstream ifs(msgStartFilePath);
  if (!ifs.good()) {
    std::cout << "Could not find saved msg starts at: " << msgStartFilePath << std::endl;
  }
  else {
    std::cout << "Loading msg starts from file: " << msgStartFilePath << std::endl;
    auto loadedJsonData = nlohmann::json::parse(ifs);
    _msgStarts = loadedJsonData["msgStarts"].get<std::vector<off_t>>();
    _timestamps = loadedJsonData["timestamps"].get<std::vector<int64_t>>();
  }

  if (_msgStarts.size() == 0) {
    std::cout << "Loading and storing msg starts..." << std::endl;
    std::system("mkdir -p ./tmp");
    std::ofstream outputJsonFile(msgStartFilePath);
    nlohmann::json jsonData = {
      {"msgStarts", nlohmann::json::array()},
      {"timestamps", nlohmann::json::array()},
    };

    int fd = open(_filePath.c_str(), O_RDONLY);
    kj::FdInputStream fdStream(fd);
    kj::BufferedInputStreamWrapper bufferedStream(fdStream);

    while (bufferedStream.tryGetReadBuffer() != nullptr) {
      const off_t msgStartPos = lseek(fd, 0, SEEK_CUR) - bufferedStream.tryGetReadBuffer().size();
      capnp::PackedMessageReader packedMsg(bufferedStream);
      auto const camSensors = packedMsg.getRoot<CapnpOutput::Frame>().getCamSensors();
      for (const auto frame: camSensors) {
        if (frame.getKey() == _name) {
          jsonData["msgStarts"].push_back(msgStartPos);
          jsonData["timestamps"].push_back(frame.getTimestamp());
          _msgStarts.push_back(msgStartPos);
          _timestamps.push_back(frame.getTimestamp());
          break;
        }
      }
    }

    outputJsonFile << jsonData.dump();
    close(fd);
    std::cout << "Saved msg starts to: " << msgStartFilePath << std::endl;
  }

  _recLength = _timestamps.back() - _timestamps.front();
  std::cout << "Length of Recording: " << _recLength * 1e-6 << " [s]" << std::endl;
}

void data_reader::RecCam::handleRequest(const std::string& requestType, const nlohmann::json& requestData, nlohmann::json& responseData) {
  // TODO: Save strings to constants to use across the code
  if (
    requestData["type"] == "client.play_rec" ||
    requestData["type"] == "client.pause_rec" ||
    requestData["type"] == "client.step_forward" ||
    requestData["type"] == "client.step_backward" ||
    requestData["type"] == "client.jump_to_ts"
  ) {
    std::unique_lock<std::mutex> lockGuard(_readLock);

    if (requestData["type"] == "client.play_rec") {
      _pause = false;
    }
    else if (requestData["type"] == "client.pause_rec") {
      _pause = true;
    }
    else if (requestData["type"] == "client.step_forward") {
      _stepForward = true;
      _pause = true;
    }
    else if (requestData["type"] == "client.step_backward") {
      if (_currFrameNr > 1) {
        _currFrameNr -= 2;
        _jumpToFrame = true;
        _pause = true;
      }
    }
    else if (requestData["type"] == "client.jump_to_ts") {
      const int64_t newTs = requestData["data"];
      // Let's assume a more or less even frame rate, with that we should get the frameNr close to the desired one
      int frameNr = (static_cast<double>(newTs) / static_cast<double>(_recLength)) * _timestamps.size();
      bool found = false;
      while (!found) {
        int64_t startTs = _timestamps[0];
        int64_t tsDiff = std::abs(_timestamps[frameNr] - startTs - newTs);
        int64_t tsDiffNext = tsDiff + 1;
        int64_t tsDiffPrevious = tsDiff + 1;
        if (frameNr + 1 < _timestamps.size()) {
          tsDiffNext = std::abs(_timestamps[frameNr+1]- startTs - newTs);
        }
        if (frameNr - 1 >= 0) {
          tsDiffPrevious = std::abs(_timestamps[frameNr-1] - startTs - newTs);
        }
        if (tsDiff < tsDiffNext && tsDiff < tsDiffPrevious) {
          found = true;
        }
        else if (tsDiffNext < tsDiffPrevious){
          frameNr++;
        }
        else {
          frameNr--;
        }
      }
      _currFrameNr = frameNr;
      _jumpToFrame = true;
      _pause = true; // Also pause recording in case it was playing
    }
    // Set rec info to app state to inform client about changes
    responseData["isPlaying"] = !_pause;
    _appState.setRecData(!_pause, _recLength, true, false);

    if (_jumpToFrame) {
      // Make sure at least one frame is finished once the actions are set to sync the reset
      // the client then send a reset command to the algo afterwards
      // Maybe not the cleanest solution, but works for now
      _finishedFrame = false;
      lockGuard.unlock();
      while (!_finishedFrame) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
    }
  }
}

void data_reader::RecCam::start() {
  _appState.setRecData(!_pause, _recLength, true);

  // Start thread to read image and store it into _currFrame
  std::thread dataReaderThread(&data_reader::RecCam::readData, this);
  dataReaderThread.detach();
}

void data_reader::RecCam::readData() {
  int64_t startSensorTs = -1;
  int64_t lastSensorTs = -1;
  auto timeLastFrameRead = std::chrono::high_resolution_clock::now();

  int fd = open(_filePath.c_str(), O_RDONLY);
  kj::FdInputStream fdStream(fd);
  auto bufferedStreamPtr = std::make_unique<kj::BufferedInputStreamWrapper>(fdStream);

  for (;;) {
    std::unique_lock<std::mutex> lockGuard(_readLock);
    if (!_pause || _stepForward || !_gotOneFrame || _jumpToFrame) {
      if (_jumpToFrame) {
        off_t newMsgPos = _msgStarts[_currFrameNr];
        bufferedStreamPtr = std::make_unique<kj::BufferedInputStreamWrapper>(fdStream);
        lseek(fd, newMsgPos, SEEK_SET);
      }

      kj::ArrayPtr<const kj::byte> framePtr = bufferedStreamPtr->tryGetReadBuffer();
      if (framePtr != nullptr) {
        _currFrameNr++;
        capnp::PackedMessageReader packedMsg(*bufferedStreamPtr);
        const auto camSensors = packedMsg.getRoot<CapnpOutput::Frame>().getCamSensors();
        for (const auto frame: camSensors) {
          if (frame.getKey() == _name) {
            // Frame syncing
            if (startSensorTs == -1) {
              startSensorTs = frame.getTimestamp();
            }
            else if (!_jumpToFrame && !_stepForward) {
              // Wait at least the time till current timestamp to sync sensor times
              std::chrono::microseconds diffSensorTs(frame.getTimestamp() - lastSensorTs);
              const auto sensorFrameEndTime = timeLastFrameRead + diffSensorTs;
              std::this_thread::sleep_until(sensorFrameEndTime);
            }
            timeLastFrameRead = std::chrono::high_resolution_clock::now();
            lastSensorTs = frame.getTimestamp();

            // Read and set img data
            const int imgWidth = frame.getImg().getWidth();
            const int imgHeight = frame.getImg().getHeight();
            // Currently expects 3 channels
            // const int channels = frame->getImg().getChannels();
            const auto rawImgData = frame.getImg().getData();
            auto bufferMat = cv::Mat(cv::Size(imgWidth, imgHeight), CV_8UC3);
            memcpy(bufferMat.data, rawImgData.begin(), rawImgData.size());
            {
              std::lock_guard<std::mutex> lockGuardRead(_readMutex);
              _currFrame = bufferMat.clone();
              _validFrame = true;
              _currTs = frame.getTimestamp() - startSensorTs;
            }
            bufferMat.release();
            _gotOneFrame = true;
            _jumpToFrame = false;
            _stepForward = false;

            break;
          }
        }
        _finishedFrame = true;
      }
      else {
        // reached end of recording
        _pause = true;
        _appState.setRecData(!_pause, _recLength, true);
      }
    }
    lockGuard.unlock();
    // Give some time to the response;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  close(fd);
}
