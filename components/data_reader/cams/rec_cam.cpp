#include "rec_cam.h"
#include <iostream>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <thread>
#include "output/frame.capnp.h"


data_reader::RecCam::RecCam(
  const std::string name,
  output::Storage& outputStorage,
  const double horizontalFov,
  const std::string recFilePath
) : 
  BaseCam(name, std::chrono::high_resolution_clock::now()),
  _recFilePath(recFilePath),
  _outputStorage(outputStorage),
  _gotOneFrame(false),
  _pause(true),
  _jumpToFrame(false)
{
  //setCamIntrinsics(width, height, horizontalFov);
  //_recLength = _timestamps.back() - _timestamps.front();
  //_frameRate = static_cast<double>(_timestamps.size()) / (static_cast<double>(_recLength) / 1000000.0);

  _outputStorage.setRecCtrlData(true, !_pause, _recLength);
}

void data_reader::RecCam::handleRequest(const std::string& requestType, const nlohmann::json& requestData, nlohmann::json& responseData) {
  std::lock_guard<std::mutex> lockGuardCtrls(_controlsMtx);
  if (requestData["type"] == "client.play_rec") {
    _pause = false;
    _outputStorage.setRecCtrlData(true, !_pause, _recLength);
  }
  else if (requestData["type"] == "client.pause_rec") {
    _pause = true;
    _outputStorage.setRecCtrlData(true, !_pause, _recLength);
  }
  else if (requestData["type"] == "client.step_forward" && _pause) {
    _stepForward = true;
  }
  else if (requestData["type"] == "client.step_backward" && _pause) {
    _newFrameNr = _currFrameNr - 1;
    _jumpToFrame = true;
  }
  else if (requestData["type"] == "client.jump_to_ts") {
    const int64_t newTs = requestData["data"];
    
    _newFrameNr = -1;

    // int64_t smallestDiff = INT64_MAX;
    // // find frame that is closest to this ts, TODO: runtime optimize the search
    // for (uint i = 0; i < _timestamps.size(); ++i)
    // {
    //   int64_t diff = std::abs(_timestamps[i] - newTs);
    //   if (i == 0 || diff < smallestDiff) {
    //     smallestDiff = diff;
    //     _newFrameNr = i;
    //   }
    // }
    _jumpToFrame = true;
    _pause = true; // Also pause recording in case it was playing
    _outputStorage.setRecCtrlData(true, !_pause, _recLength);
  }
}

void data_reader::RecCam::start() {
  // Start thread to read image and store it into _currFrame
  std::thread dataReaderThread(&data_reader::RecCam::readData, this);
  dataReaderThread.detach();
}

void data_reader::RecCam::readData() {
  int64_t startSensorTs = -1;
  int64_t lastSensorTs = -1;
  auto timeLastFrameRead = std::chrono::high_resolution_clock::now();

  int fd = open(_recFilePath.c_str(), O_RDONLY);
  kj::FdInputStream fdStream(fd);
  kj::BufferedInputStreamWrapper bufferedStream(fdStream);
  while (bufferedStream.tryGetReadBuffer() != nullptr) {
    std::cout << "HERE" << std::endl;
    capnp::PackedMessageReader message(bufferedStream);
    const auto frame = message.getRoot<CapnpOutput::Frame>();
    const auto frameLen = frame.getPlannedFrameLength(); // in [ms]
    const auto camSensors = frame.getCamSensors();
    for (int i = 0; i < camSensors.size(); ++i) {
      if (camSensors[i].getKey() == _name) {
        if (startSensorTs == -1) {
          startSensorTs = camSensors[i].getTimestamp();
          lastSensorTs = camSensors[i].getTimestamp();
        }
        else {
          // Wait at least the time till current timestamp to sync sensor times
          std::chrono::microseconds diffSensorTs(camSensors[i].getTimestamp() - lastSensorTs);
          auto diffSensorTs2 = std::chrono::duration<double, std::micro>(camSensors[i].getTimestamp() - lastSensorTs);
          std::cout << diffSensorTs2.count() << std::endl;
          std::cout << diffSensorTs.count() << std::endl;
          const auto sensorFrameEndTime = timeLastFrameRead + diffSensorTs;
          std::this_thread::sleep_until(sensorFrameEndTime);
        }

        timeLastFrameRead = std::chrono::high_resolution_clock::now();
        std::cout << "Reading frame: " << camSensors[i].getTimestamp() << std::endl;
      }
    }
    break;
  }

  // for (;;) {
    // if (!_pause || _stepForward || !_gotOneFrame || _jumpToFrame) {
    //   if (_jumpToFrame && _newFrameNr >= 0) {
    //     // Algo also has a requestHandler and resets Algo on jump_to_ts and step_backward
    //     // std::lock_guard<std::mutex> lockGuardCtrls(_controlsMtx);
    //     // _stream.set(cv::CAP_PROP_POS_FRAMES, _newFrameNr);
    //   }

      // const int currFrameNr = _stream.get(cv::CAP_PROP_POS_FRAMES);
      // const bool success = _stream.read(_bufferFrame);
      // if (success) {
      //   std::lock_guard<std::mutex> lockGuardRead(_readMutex);
      //   _currFrame = _bufferFrame.clone();
      //   _bufferFrame.release();
      //   _validFrame = success;

      //   std::lock_guard<std::mutex> lockGuardCtrls(_controlsMtx);
      //   if (_timestamps.size() == 0) {
      //     const double tsMsec = _stream.get(cv::CAP_PROP_POS_MSEC);
      //     _currTs = static_cast<int64>(tsMsec * 1000.0);
      //     _currFrameNr = currFrameNr;
      //   }
      //   else {
      //     _currTs = _timestamps.at(currFrameNr);
      //     _currFrameNr = currFrameNr;
      //   }
      // }

      // Wait the amount of time to the next timestamp or one frame length if timestamps are not available
      // TODO: Wait this time from the beginning of the frame including the fetching of the frame and other meta data
      // TODO: There is still something wrong when approching the end of the recording and going beyond, it does not stop
      // int64_t waitTimeUs = 0;
      // if (_timestamps.size() > (currFrameNr + 1)) {
      //   waitTimeUs = _timestamps.at(currFrameNr + 1) - _timestamps.at(currFrameNr);
      // }
      // else {
      //   waitTimeUs = (1 / _frameRate) * 1000000;
      // }
      // std::this_thread::sleep_for(std::chrono::microseconds(5000));

      // _gotOneFrame = true;
      // {
      //   std::lock_guard<std::mutex> lockGuardCtrls(_controlsMtx);
      //   _stepForward = false;
      //   _jumpToFrame = false;

      //   if (_currTs >= _recLength) {
      //     _pause = true;
      //     _outputStorage.setRecCtrlData(true, !_pause, _recLength);
      //   }
      // }
    // }
  // }
}
