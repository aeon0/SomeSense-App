#pragma once

#include <fstream>
#include <atomic>
#include <tuple>
#include "irec.h"
#include "opencv2/opencv.hpp"
#include "frame.pb.h"


namespace data {
  class PbRec : public IRec {
  public:
    PbRec(std::string binPath, std::string metaPath);
    void fillFrame(proto::Frame& frame) override;
    int64_t getRecLength() const override { return _recLength; }
    void reset() override;
    void setRelTs(int64_t newRelTs) override;

  private:
    std::vector<off64_t> _msgStart; // protobuf msg starts in bytes
    std::vector<int64_t> _timestamp; // absTs in [us]
    int64_t _recLength; // length of recording in [us]

    size_t _msgSize;
    char* _buffer;
    std::ifstream _stream;

    std::mutex _readLock;
  };
}
