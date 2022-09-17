#pragma once

#include <memory>
#include "irequest_listener.h"


namespace com_out {
  class IRequestHandler {
  public:
    virtual void registerRequestListener(std::shared_ptr<IRequestListener> listener) = 0;
    virtual void deleteRequestListener(std::shared_ptr<IRequestListener> listener) = 0;
  };
}
