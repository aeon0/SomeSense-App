#include <ecal/ecal.h>
#include <ecal/msg/string/publisher.h>
#include <ecal/msg/protobuf/publisher.h>
#include <iostream>
#include <thread>
#include "frame.pb.h"


int main(int argc, char** argv) {
  std::cout << "** Start eCAL Node **" << std::endl;

  // Setup eCAL communication
  eCAL::Initialize(argc, argv, "eCAL Node");
  eCAL::protobuf::CPublisher<proto::Frame> pub("somesense_app");

  while (eCAL::Ok())
  {
    proto::Frame frame;
    pub.Send(frame);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  eCAL::Finalize();
  std::cout << std::endl << "** Exit eCAL Node **" << std::endl;
  return 0;
}
