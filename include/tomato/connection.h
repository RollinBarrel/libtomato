#pragma once

#include "message.h"
#include "socket.h"

#define TOMATO_CONNECTION_BUFSIZE (1024 * 32)

namespace Tomato {
class Connection {
public:
  enum ConnectionState {
    STATE_OFFLINE,
    STATE_CONNECTING,
    STATE_AWAITING_HELLO,
    STATE_AWAITING_HELLO_RESP,
    STATE_AWAITING_SIGNALLIST,
    STATE_OK
  };

  static int InitSockets();
  static int SocketSetNonBlocking(SOCKET handle);
  static int SocketSetKeepalive(SOCKET handle);

private:
  int InitSocket();

public:
  bool IsClient;
  char IBuf[TOMATO_CONNECTION_BUFSIZE];
  char OBuf[TOMATO_CONNECTION_BUFSIZE];
  ConnectionState State = STATE_OFFLINE;
  SOCKET Socket;

  Msg::SignalList SignalList;

  Connection() { IsClient = true; }
  Connection(SOCKET socket) {
    Socket = socket;
    IsClient = false;
  }

  virtual int Shutdown();

  virtual int OnConnected() { return 0; }
  virtual int ProcessHello(Msg::HelloReq *req) { return 0; }
  virtual int ProcessHelloResp(Msg::HelloResp *resp) { return 0; }
  virtual int ProcessSignalList() { return 0; }
  virtual int ProcessSignal(Msg::Signal *signal) { return 0; }

  int Connect(const char *ip, int port, bool force = false);
  virtual int Process();
  int Send(size_t size);
  int SendSignal(Msg::Signal *signal);
};
} // namespace Tomato