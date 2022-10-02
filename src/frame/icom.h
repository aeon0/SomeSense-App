#pragma once
#include <functional>
#include <string>
#include "frame.pb.h"
#include "recmeta.pb.h"


namespace frame {
  class ICom {
  public:
    typedef std::function<void(const std::string& request, std::string& response)> CallbackT;

    virtual void registerClientCallback(CallbackT cb) = 0;
    virtual void sendFrame(const proto::Frame& msg) = 0;
    virtual void syncFrame(const proto::Frame& msg) = 0;
    virtual void sendRecMeta(const proto::RecMeta& msg) = 0;

    virtual bool isOk() const { return true; }
    virtual void finalize() { }
  };
}
