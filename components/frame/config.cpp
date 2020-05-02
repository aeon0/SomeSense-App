#include "config.h"
#include "serialize/frame.capnp.h"


const int Config::interfaceVersionMajor = INTERFACE_VERSION_MAJOR;
const int Config::interfaceVersionMinor = INTERFACE_VERSION_MINOR;
const double Config::goalFrameLength = 50.0; // 50 ms -> 20 fps
const std::string Config::storagePath = "storage_data/";
