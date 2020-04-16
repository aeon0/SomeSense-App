#pragma once

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/tcp.h>
#include <sys/un.h>

#include "server.h"


namespace com_out {
  class TcpServer : public Server {
  public:
    TcpServer(serialize::AppState& appState);

  protected:
    void create();
    void closeSocket();
  };
}
