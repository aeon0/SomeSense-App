#pragma once

#include "irequest_listener.h"
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string>
#include <vector>
#include <memory>


namespace com_out {
  class Server {
  public:
    Server();
    ~Server();

    void run();
    void stop();
    bool broadcast(const std::string msg) const;

    void registerRequestListener(std::shared_ptr<IRequestListener> listener);
    void deleteRequestListener(std::shared_ptr<IRequestListener> listener);

  protected:
    virtual void create() = 0;
    virtual void closeSocket() = 0;
    void serve();
    void handle(int client);
    std::string getRequest(int client);
    bool sendToClient(int client, const std::string msg) const;

    int _server;
    char* _buf;
    std::vector<int> _clients;

    std::vector<std::shared_ptr<IRequestListener>> _requestListeners;
  };
}

