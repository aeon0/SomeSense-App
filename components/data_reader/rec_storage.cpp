#include "rec_storage.h"
#include <chrono>
#include <sys/stat.h>
#include <sys/types.h>


data_reader::RecStorage::RecStorage(const std::string storageBasePath, const SensorStorage& sensorStorage) :
  _isStoring(false), _storageBasePath(storageBasePath), _sensorStorage(sensorStorage) {}

std::string data_reader::RecStorage::formatTimePoint(std::chrono::system_clock::time_point point) {
    static_assert(std::chrono::system_clock::time_point::period::den == 1000000000 && std::chrono::system_clock::time_point::period::num == 1);
    std::string out(29, '0');
    char* buf = &out[0];
    std::time_t now_c = std::chrono::system_clock::to_time_t(point);
    std::strftime(buf, 21, "%Y-%m-%dT%H:%M:%S.", std::localtime(&now_c));
    sprintf(buf+20, "%09ld", point.time_since_epoch().count() % 1000000000);
    return out;
}

void data_reader::RecStorage::startStoring() {
  if (!_isStoring) {
    _isStoring = true;

    // Create a new folder for storing the data based on the current date + time
    std::ostringstream folderName;
    _currentStoragePath = _storageBasePath + formatTimePoint(std::chrono::system_clock::now()) + "/";
    mkdir(_currentStoragePath.c_str(), 0755);

    // Loop through all sensors in the storage and create e.g. cv::VideoWriter for each camera
    for (auto const& [key, cam]: _sensorStorage.getCams()) {
      const double fps = cam->getFrameRate();
      const cv::Size frameSize = cam->getFrameSize();
      const std::string videoFilePath = _currentStoragePath + cam->getName() + "_" + key + ".mp4";

      // .mp4 => 'm','p','4','v' || 'F','M','P','4' || 'X','2','6','4' || 'H','2','6','4'
      // .avi => 'M','J','P','G'
      auto writer = cv::VideoWriter(videoFilePath, cv::VideoWriter::fourcc('m','p','4','v'), fps, frameSize);
      _videoWriters.insert({key, writer});
    }
  }
}

void data_reader::RecStorage::stopStoring() {
  if (_isStoring) {
    _isStoring = false;
    for (auto& [key, writer]: _videoWriters) {
      writer.release();
    }
    _videoWriters.clear();
  }
}

void data_reader::RecStorage::saveFrame() {
  if (_isStoring) {
    for (auto& [key, writer]: _videoWriters) {
      // TODO: Not sure how to handle the ts of the frames...
      auto [success, ts, frame] = _sensorStorage.getCams().at(key)->getFrame();
      if (success) {
        writer.write(frame);
      }
    }
  }
}
