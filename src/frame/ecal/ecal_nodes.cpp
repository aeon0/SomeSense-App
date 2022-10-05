#include "ecal_nodes.h"


frame::EcalNodes::EcalNodes() :
  _publisherFrame("somesense_app"),
  _publisherSyncFrame("somesense_app_sync"),
  _publisherRecMeta("somesense_app_recmeta"),
  _server("somesense_server"),
  _cb(nullptr)
{
  // // Make them tcp connections
  // _publisherFrame.SetLayerMode(eCAL::TLayer::tlayer_udp_mc, eCAL::TLayer::smode_off);
  // _publisherFrame.SetLayerMode(eCAL::TLayer::tlayer_tcp, eCAL::TLayer::smode_auto);

  // _publisherSyncFrame.SetLayerMode(eCAL::TLayer::tlayer_udp_mc, eCAL::TLayer::smode_off);
  // _publisherSyncFrame.SetLayerMode(eCAL::TLayer::tlayer_tcp, eCAL::TLayer::smode_auto);

  // _publisherRecMeta.SetLayerMode(eCAL::TLayer::tlayer_udp_mc, eCAL::TLayer::smode_off);
  // _publisherRecMeta.SetLayerMode(eCAL::TLayer::tlayer_tcp, eCAL::TLayer::smode_auto);

  using namespace std::placeholders;
  _server.AddMethodCallback("frame_ctrl", "", "", std::bind(&EcalNodes::onMethod, this, _4, _5));
}

int frame::EcalNodes::onMethod(const std::string& request, std::string& response) {
  std::lock_guard<std::mutex> lock(_cb_sync);
  if (_cb) {
    (_cb)(request, response);
  }
  return 0;
}

void frame::EcalNodes::registerClientCallback(CallbackT cb) {
  std::lock_guard<std::mutex> lock(_cb_sync);
  _cb = cb;
};

void frame::EcalNodes::sendFrame(const proto::Frame& msg) {
  _publisherFrame.Send(msg);
}

void frame::EcalNodes::syncFrame(const proto::Frame& msg) {
  _publisherSyncFrame.Send(msg);
}

void frame::EcalNodes::sendRecMeta(const proto::RecMeta& msg) {
  _publisherRecMeta.Send(msg);
}
