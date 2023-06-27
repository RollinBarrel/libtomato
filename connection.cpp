#include <tomato/connection.h>

#include <iostream>

namespace Tomato {
int Connection::InitSockets() {
#if _WIN32
  WSADATA wsaData;
  int err = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (err != NO_ERROR) {
    std::cerr << "WSAStartup err " << err << std::endl;
    return -1;
  }
#endif
  return 0;
}

int Connection::SocketSetNonBlocking(SOCKET handle) {
#if _WIN32
  unsigned long nonblock = 1;
  if (ioctlsocket(handle, FIONBIO, &nonblock)) {
    std::cerr << "ioctlsocket err " << SOCKERROR << std::endl;
    return -1;
  }
#else
  int flags = fcntl(handle, F_GETFL, 0);
  if (flags == -1) {
    std::cerr << "fcntrl F_GETFL err " << errno << std::endl;
    return -1;
  }

  if (fcntl(handle, F_SETFL, flags | O_NONBLOCK)) {
    std::cerr << "fcntrl F_SETFL err " << errno << std::endl;
    return -2;
  }
#endif

  return 0;
}
int Connection::SocketSetKeepalive(SOCKET handle) {
  int opt = 1;
  return setsockopt(handle, SOL_SOCKET, SO_KEEPALIVE, (char *)&opt,
                    sizeof(opt));
}

int Connection::InitSocket() {
  int err = InitSockets();
  if (err < 0)
    return err;

  Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (SOCKET_IS_INVALID(Socket)) {
    std::cerr << "socket err " << SOCKERROR << std::endl;
    return -2;
  }

  err = SocketSetNonBlocking(Socket);
  if (err < 0)
    return -2 + err;

  err = SocketSetKeepalive(Socket);
  if (err < 0)
    return -4;
  return 0;
}

int Connection::Shutdown() {
#if _WIN32
  closesocket(Socket);
#else
  close(Socket);
#endif
  Socket = -1;
  State = STATE_OFFLINE;
  return 0;
}

int Connection::Connect(const char *ip, int port, bool force) {
  if (!force) {
    if (State == STATE_CONNECTING) {
      int err;
#if _WIN32
      int errSize;
#else
      socklen_t errSize;
#endif
      errSize = sizeof(err);
      if (getsockopt(Socket, SOL_SOCKET, SO_ERROR,
                     reinterpret_cast<char *>(&err), &errSize)) {
        std::cerr << "getsockopt err " << SOCKERROR << std::endl;
        Shutdown();
        return -1;
      }

      if (!err) {
        OnConnected();
        return 0;
      }

      if (err == SOCKET_INPROGRESS || err == SOCKET_ALREADY)
        return 0;

      if (err == SOCKET_CONNREFUSED) {
        Shutdown();
        return -2;
      }

      std::cerr << "connect err " << err << std::endl;
      Shutdown();
      return -3;
    } else {
      if (State != STATE_OFFLINE)
        return 0;
    }
  } else {
    if (State != STATE_OFFLINE)
      Shutdown();
  }

  InitSocket();

  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(32786);
  addr.sin_addr.s_addr = inet_addr(ip);

  State = STATE_CONNECTING;
  if (connect(Socket, (struct sockaddr *)&addr, sizeof(addr))) {
    if (SOCKERROR == SOCKET_INPROGRESS || SOCKERROR == SOCKET_WOULDBLOCK)
      return 0;

    std::cerr << "connect err " << SOCKERROR << std::endl;
    Shutdown();
    return -4;
  }

  OnConnected();
  return 0;
}

int Connection::Process() {
  if (State == STATE_OFFLINE || State == STATE_CONNECTING)
    return 0;

  size_t offset = 0;
  while (true) {
    int len = recv(Socket, &Connection::IBuf[offset],
                   TOMATO_CONNECTION_BUFSIZE - offset, 0);
    if (len == -1) {
      if (SOCKERROR == SOCKET_WOULDBLOCK)
        return 0;

      std::cerr << "recv err " << SOCKERROR << std::endl;
      Shutdown();
      return -1;
    }

    if (len == 0) {
      Shutdown();
      return -2;
    }

    len += offset;

    Msg::StreamBuffer buffer(Connection::IBuf, len);
    std::istream input(&buffer);

    size_t pos = 0;
    while (pos < len) {
      switch (State) {
      case STATE_AWAITING_HELLO: {
        Msg::HelloReq req;
        if (!req.Read(&input))
          ProcessHello(&req);
        break;
      }
      case STATE_AWAITING_HELLO_RESP: {
        Msg::HelloResp resp;
        if (!resp.Read(&input))
          ProcessHelloResp(&resp);
        break;
      }
      case STATE_AWAITING_SIGNALLIST: {
        if (!SignalList.Read(&input))
          ProcessSignalList();
        break;
      }
      case STATE_OK: {
        Msg::Signal signal;
        signal.IsOut = !IsClient;
        signal.SignalList = &SignalList;
        if (!signal.Read(&input))
          ProcessSignal(&signal);
        break;
      }
      case STATE_CONNECTING:
      case STATE_OFFLINE:
        break;
      }
      pos = input.tellg();
#if _WIN32
      u_long can_recv;
      ioctlsocket(
#else
      int can_recv;
      ioctl(
#endif
          Socket, FIONREAD, &can_recv);
      if (can_recv > 0) { // can theoretically become a bottleneck
        offset = len - pos;
        memmove(Connection::IBuf, &Connection::IBuf[pos], offset);
        break;
      }
    }
  }

  return 0;
}

int Connection::Send(size_t size) {
  int sent = 0;
  while (sent != size) {
    int len =
        send(Socket, &Connection::OBuf[sent], size - sent, SOCKET_NOSIGNAL);
    if (len == -1) {
      if (SOCKERROR == SOCKET_WOULDBLOCK)
        continue;

      std::cerr << "send err " << SOCKERROR << std::endl;
      Shutdown();
      return -1;
    }

    sent += len;
  }
  return 0;
}

int Connection::SendSignal(Msg::Signal *signal) {
  Msg::StreamBuffer buffer(Connection::OBuf, TOMATO_CONNECTION_BUFSIZE);
  std::ostream output(&buffer);

  signal->Write(&output);

  size_t size = output.tellp();
  if (Send(size))
    return -1;

  return 0;
}

} // namespace Tomato