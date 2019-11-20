#include "adder.h"
#include "data_reader/cams/icam.h"
#include "data_reader/cams/usb_cam.h"
#include <iostream>


int main() {
  Adder adder = Adder();
  int added_value = adder.add(2, 4);
  std::cout << added_value << std::endl;
  std::cout << "SOME TEST" << std::endl;

  ICam* cam = new UsbCam();
  cam->getFrame();

  return 0;
}
