#include "pb_rec.h"
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <chrono>
#include "util/json.hpp"
#include <google/protobuf/io/zero_copy_stream.h>


data::PbRec::PbRec(std::string binPath, std::string metaPath)
{
  std::ifstream ifs(metaPath);
  if (!ifs.good()) {
    std::runtime_error("Could not open file:  " + metaPath);
  }
  std::cout << "Loading metadata from file: " << metaPath << std::endl;
  auto jsonData = nlohmann::json::parse(ifs);
  _msgStart = jsonData["msgStart"].get<std::vector<off64_t>>();
  _timestamp = jsonData["timestamp"].get<std::vector<int64_t>>();
  _recLength = _timestamp.back() - _timestamp.front();

  _msgSize = jsonData["msgSize"].get<size_t>();
  _buffer = new char[_msgSize];
  _stream = std::ifstream(binPath);
}

void data::PbRec::reset() {
  _stream.seekg(0);
}

void data::PbRec::setRelTs(int64_t newRelTs) {
  int frameNr = (static_cast<double>(newRelTs) / static_cast<double>(_recLength)) * _timestamp.size();
  bool found = false;
  int64_t startTs = _timestamp[0];
  while (!found) {
    int64_t tsDiff = std::abs(_timestamp[frameNr] - startTs - newRelTs);
    int64_t tsDiffNext = tsDiff + 1;
    int64_t tsDiffPrevious = tsDiff + 1;
    if (frameNr + 1 < _timestamp.size()) {
      tsDiffNext = std::abs(_timestamp[frameNr+1]- startTs - newRelTs);
    }
    if (frameNr - 1 >= 0) {
      tsDiffPrevious = std::abs(_timestamp[frameNr-1] - startTs - newRelTs);
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
  auto newMsgPos = _msgStart[frameNr];
  _stream.seekg(newMsgPos);
}

void data::PbRec::fillFrame(proto::Frame& frame) {
  _stream.read(_buffer, _msgSize);
  frame.ParseFromArray(_buffer, _msgSize);
  // Adapt the relative timestamp
  auto x = frame.absts();
  auto newRelTs = frame.absts() - _timestamp.front();
  frame.set_relts(newRelTs);
}
