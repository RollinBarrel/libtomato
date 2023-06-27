#pragma once

#include <cstdint>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace Tomato {
namespace Msg {
enum MsgType { TMSG_HELLO_REQ, TMSG_HELLO_RESP, TMSG_SIGNALLIST };
enum ValType { TMSG_INT = 0, TMSG_BOOL = 1, TMSG_FLOAT = 2, TMSG_ENUM = 3 };

typedef struct LanguageCode {
  char Code[2];

  LanguageCode() {}
  LanguageCode(char l1, char l2) {
    Code[0] = l1;
    Code[1] = l2;
  }

  friend bool operator==(const LanguageCode &l, const LanguageCode &r) {
    return l.Code[0] == r.Code[0] && l.Code[1] == r.Code[1];
  }
} LanguageCode;

struct StreamBuffer : public std::streambuf {
  StreamBuffer(char *s, std::size_t n) {
    setg(s, s, s + n);
    setp(s, s + n);
  }

  pos_type seekoff(off_type off, std::ios_base::seekdir dir,
                   std::ios_base::openmode which) {
    if (which == std::ios_base::in) {
      return gptr() - eback();
    } else if (which == std::ios_base::out) {
      return pptr() - pbase();
    }

    return -1;
  }
};

class Message {
public:
  virtual int Read(std::istream *input) = 0;
  virtual int Write(std::ostream *output) = 0;
};

typedef struct EnumDescVal {
  std::string Name;
  std::string Description;
} EnumDescVal;

class EnumDesc : public Message {
public:
  std::string Name;
  std::string Description;
  std::vector<EnumDescVal> Values;

  int Read(std::istream *input);
  int Write(std::ostream *output);
};

typedef struct SignalDescArg {
  uint8_t Type;
  union {
    uint8_t EnumId;
  } Param;
  std::string Name;
  std::string Description;
} SignalDescArg;

class SignalDesc : public Message {
public:
  std::string Name;
  std::string Description;
  std::vector<SignalDescArg> Args;

  int Read(std::istream *input);
  int Write(std::ostream *output);
};

class SignalList : public Message {
public:
  bool Initialized = false;
  std::vector<EnumDesc> Enums;
  std::vector<SignalDesc> In;
  std::vector<SignalDesc> Out;

  int Read(std::istream *input);
  int Write(std::ostream *output);
};

class Signal : public Message {
  typedef struct Arg {
    union {
      int32_t Int;
      float Float;
      char Chars[4];
    };
  } Arg;

public:
  bool IsOut;
  uint16_t Index;
  std::vector<Arg> Args;

  Tomato::Msg::SignalList *SignalList;

  int Read(std::istream *input);
  int Write(std::ostream *output);
};

class HelloReq : public Message {
  char Magic[10] = {'G', 'Y', 'R', 'O', 'T', 'O', 'M', 'A', 'T', 'O'};

public:
  uint32_t MessageVersion = 1;
  uint32_t ClientVersion;
  std::string Title;
  std::vector<LanguageCode> Languages;

  int Read(std::istream *input);
  int Write(std::ostream *output);
};

class HelloResp : public Message {
  char Magic[9] = {'T', 'O', 'M', 'A', 'T', 'O', 'O', 'K', '!'};

public:
  LanguageCode Language;

  int Read(std::istream *input);
  int Write(std::ostream *output);
};

} // namespace Msg
} // namespace Tomato