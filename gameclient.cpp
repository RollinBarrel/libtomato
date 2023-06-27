#include <tomato/gameclient.h>

namespace Tomato {
int GameClient::SendHello() {
  Msg::StreamBuffer buffer(Connection::OBuf, TOMATO_CONNECTION_BUFSIZE);
  std::ostream output(&buffer);
  size_t stream_begin = output.tellp();

  Msg::HelloReq req;
  req.ClientVersion = ClientVersion;
  req.Title = Title;

  Msg::LanguageCode en;
  en.Code[0] = 'e';
  en.Code[1] = 'n';
  req.Languages.emplace_back(en);

  req.Write(&output);

  size_t size = (size_t)output.tellp() - stream_begin;
  if (Send(size))
    return -1;

  State = STATE_AWAITING_HELLO_RESP;
  return 0;
}

int GameClient::ProcessHelloResp(Msg::HelloResp *resp) {
  Language = resp->Language;

  SendSignalList();
  return 0;
}

int GameClient::SendSignalList() {
  if (!SignalList.Initialized)
    return -1;

  Msg::StreamBuffer buffer(Connection::OBuf, TOMATO_CONNECTION_BUFSIZE);
  std::ostream output(&buffer);
  size_t stream_begin = output.tellp();

  SignalList.Write(&output);

  size_t size = (size_t)output.tellp() - stream_begin;
  if (Send(size))
    return -2;

  State = STATE_OK;
  return 0;
}

int GameClient::OnConnected() {
  SendHello();
  return 0;
}
} // namespace Tomato