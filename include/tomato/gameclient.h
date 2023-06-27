#pragma once

#include "connection.h"

namespace Tomato {
class GameClient : public Connection {
protected:
  std::string Title;
  uint32_t ClientVersion;
  Msg::LanguageCode Language;

  int SendHello();
  int SendSignalList();

  int ProcessHelloResp(Msg::HelloResp *resp);

public:
  GameClient(std::string title, uint32_t client_version) {
    Title = title;
    ClientVersion = client_version;
  }

  int OnConnected();
};
} // namespace Tomato