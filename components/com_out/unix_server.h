#pragma once

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/un.h>

#include "server.h"


namespace com_out {
  class UnixServer : public Server {
  public:
    UnixServer();

  protected:
    void create();
    void closeSocket();

  private:
    static void interrupt(int);
    
    static const char* _socketName;
  };
}
