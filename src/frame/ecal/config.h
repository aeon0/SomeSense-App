#include <string.h>


namespace config {
  const int INTERFACE_VERSION_MAJOR = 0;
  const int INTERFACE_VERSION_MINOR = 1;
  const double GOAL_FRAME_LENGTH = 50.0; // 50 ms -> 20 fps

  const std::string SERVER_METHOD_FRAME_CTRL = "frame_ctrl";
  const std::string SERVER_SERVICE_NAME = "somesense_server";
  const std::string PUBLISHER_NAME_APP = "somesense_app";
  const std::string PUBLISHER_NAME_APP_SYNC = "somesense_app_sync";
  const std::string PUBLISHER_NAME_RECMETA = "somesense_recmeta";
}
