#pragma once

#include "irequest_listener.h"
#include "output/storage.h"
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

typedef unsigned char BYTE;


namespace com_out {
  class Server {
  public:
    Server(const output::Storage& outputStorage);
    ~Server();

    void run();
    void stop();

    void pollOutput();

    void registerRequestListener(std::shared_ptr<IRequestListener> listener);
    void deleteRequestListener(std::shared_ptr<IRequestListener> listener);

  protected:
    virtual void create() = 0;
    virtual void closeSocket() = 0;

    void serve();
    void handle(int client);
    std::string getRequest(int client);
    bool sendToClient(int client, const BYTE* buf, const int len) const;

    // Broadcast different type of messages to all clients
    void broadcast(const std::string payload) const;
    void broadcast(int sensorIdx, BYTE* payload, int width, int height, int channels, int64_t ts) const;
    // Convert different type of payloads to a complete message with header
    std::tuple<BYTE*, int> createMsg(const std::string payload) const;
    std::tuple<BYTE*, int> createMsg(int sensorIdx, BYTE* payload, int width, int height, int channels, int64_t ts) const;

    int _server;
    char* _buf;
    std::vector<int> _clients;

    std::vector<std::shared_ptr<IRequestListener>> _requestListeners;

    static const int _headerSize = 20;

    const output::Storage& _outputStorage;
    int64_t _lastSentTs;
    bool _pollOutput;
  };
}
