#pragma once

#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string>


namespace com_out {
  class Server {
  public:
    Server();
    ~Server();

    void run();

  protected:
    virtual void create() = 0;
    virtual void closeSocket() = 0;
    void serve();
    void handle(int client);
    std::string getRequest(int client);
    bool sendResponse(int client, std::string response);

    int _server;
    int _buflen;
    char* _buf;
  };
}

