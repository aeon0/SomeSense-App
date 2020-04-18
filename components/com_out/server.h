#pragma once

#include "irequest_handler.h"
#include "serialize/app_state.h"
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <mutex>

#include <string>
#include <vector>
#include <memory>

typedef unsigned char BYTE;


namespace com_out {
  class Server : public IRequestHandler {
  public:
    Server(serialize::AppState& appState);
    ~Server();

    void run();
    void stop();

    void pollOutput();

    void registerRequestListener(std::shared_ptr<IRequestListener> listener) override;
    void deleteRequestListener(std::shared_ptr<IRequestListener> listener) override;

  protected:
    static const int _headerSize = 16;
    static const int _bufSize = 1024;

    virtual void create() = 0;
    virtual void closeSocket() = 0;

    void serve();
    void handle(int client);
    std::string getRequest(int client);
    bool sendToClient(int client, const BYTE* buf, const int len);

    // Broadcast different type of messages to all clients
    void broadcast(const std::string payload);
    void broadcast(const BYTE* payload, const int payloadSize);
    // Convert different type of payloads to a complete message with header
    std::tuple<BYTE*, int> createMsg(const std::string payload) const; // json
    std::tuple<BYTE*, int> createMsg(const BYTE* payload, const int payloadSize) const; // capnp

    int _server;
    char* _buf;
    std::mutex _clientsMtx;
    std::vector<int> _clients;

    std::vector<std::shared_ptr<IRequestListener>> _requestListeners;

    serialize::AppState& _appState;
    int64_t _lastSentTs;
    bool _pollOutput;

    std::mutex _newClientMtx;
    bool _newClient; // in case there is a new client, this is true (false after next broadcast)
  };
}
