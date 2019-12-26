#pragma once

#include <string>
#include "utilities/json.hpp"


// In case an object should listen to messages to the server, it needs to implement this
// and register itself as a request listener to the Server object
namespace com_out {
  class IRequestListener {
  public:
    virtual void handleRequest(const std::string& requestType, const nlohmann::json& requestData, nlohmann::json& responseData) = 0;
  };
}
