#pragma once
#include <ecal/ecal.h>
#include <ecal/msg/string/publisher.h>
#include <ecal/msg/protobuf/publisher.h>
#include <ecal/ecal_server.h>

#include <iostream>
#include <thread>

#include "../icom.h"


namespace frame {
  static void initEcal() {
    std::cout << "** Start eCAL Node **" << std::endl;
    eCAL::Initialize(0, nullptr, "eCAL Node");
  };

  class EcalNodes: public ICom {
  public:
    EcalNodes();

    void registerClientCallback(CallbackT cb) override;
    void sendFrame(const proto::Frame& msg) override;
    void syncFrame(const proto::Frame& msg) override;
    void sendRecMeta(const proto::RecMeta& msg) override;

    bool isOk() const override { return eCAL::Ok(); }
    void finalize() override { eCAL::Finalize(); }

  private:
    int onMethod(const std::string& request, std::string& response);

    CallbackT _cb;

    eCAL::protobuf::CPublisher<proto::Frame> _publisherFrame;
    eCAL::protobuf::CPublisher<proto::Frame> _publisherSyncFrame;
    eCAL::protobuf::CPublisher<proto::RecMeta> _publisherRecMeta;
    eCAL::CServiceServer _server;

    std::mutex _cb_sync;
  };
}
