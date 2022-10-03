#pragma once
#include "../icom.h"
#include <thread>
#include <mutex>
#include <string>


typedef unsigned char BYTE;

namespace frame {
  enum TcpMessageType {
    UNDEFINED = 0,
    JSON,
    PROTO_FRAME,
    PROTO_SYNC,
    PROTO_RECMETA
  };

  class TcpServer: public ICom {
  public:
    TcpServer();
    ~TcpServer();

    void registerClientCallback(CallbackT cb) override;
    void sendFrame(const proto::Frame& msg) override;
    void syncFrame(const proto::Frame& msg) override;
    void sendRecMeta(const proto::RecMeta& msg) override;

    bool isOk() const override { return true; }
    void finalize() override { }

  private:
    template<typename T>
    void sendProto(const T& msg, TcpMessageType type) {
      size_t size = msg.ByteSizeLong();

      // With vector
      // _protoMsgBuf.resize(msg.ByteSize());
      // msg.SerializeToArray((void*)&_protoMsgBuf[0], (int)size);
      // broadcast(&_protoMsgBuf[0], size, frame::PROTO);

      // With c char ptr
      void* buffer = malloc(size);
      msg.SerializeToArray(buffer, size);
      broadcast((BYTE*)buffer, size, type);
      free(buffer);
    }

    void serve();
    void handle(int client);
    std::string getRequest(int client);

    bool sendToClient(int client, const BYTE* buf, const int len);
    void broadcast(const BYTE* payload, const int payloadSize, TcpMessageType type);
    std::tuple<BYTE*, int> createMsg(const BYTE* payload, const int payloadSize, TcpMessageType type) const;

    static const int _PORT = 8999;
    static const int _HEADER_SIZE = 16;
    static const int _BUFFER_SIZE = 1024;

    std::thread _thread;
    int _server;
    char* _buf;
    std::mutex _clientsMtx;
    std::vector<int> _clients;

    CallbackT _cb;
    std::mutex _cb_sync;
    std::vector<BYTE> _protoMsgBuf;
  };
}
