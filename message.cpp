#include <tomato/message.h>

#include <cstring>
#include <vector>

namespace Tomato {
namespace Msg {
int HelloReq::Read(std::istream *input) {
  char magic[sizeof(Magic)];
  input->read(magic, sizeof(magic));
  int diff = std::strncmp(magic, Magic, sizeof(magic));
  if (diff != 0)
    return -1;

  uint32_t message_version;
  input->read(reinterpret_cast<char *>(&message_version),
              sizeof(message_version));
  if (message_version != MessageVersion)
    return -2;

  input->read(reinterpret_cast<char *>(&ClientVersion), sizeof(ClientVersion));

  uint8_t title_length = 0;
  input->read(reinterpret_cast<char *>(&title_length), sizeof(title_length));
  if (!title_length)
    return -3;

  Title = std::string(title_length, '\0');
  input->read(Title.data(), title_length);

  uint8_t language_count = 0;
  input->read(reinterpret_cast<char *>(&language_count),
              sizeof(language_count));
  if (!language_count)
    return -4;
  Languages.reserve(language_count);
  for (int i = 0; i < language_count; ++i) {
    LanguageCode language = {};
    input->read(reinterpret_cast<char *>(&language.Code),
                sizeof(language.Code));
    Languages.push_back(language);
  }

  return 0;
}

int HelloReq::Write(std::ostream *output) {
  output->write(reinterpret_cast<char *>(Magic), sizeof(Magic));

  output->write(reinterpret_cast<char *>(&MessageVersion),
                sizeof(MessageVersion));

  output->write(reinterpret_cast<char *>(&ClientVersion),
                sizeof(ClientVersion));

  uint8_t title_length = Title.length();
  output->write(reinterpret_cast<char *>(&title_length), sizeof(title_length));
  output->write(Title.c_str(), title_length);

  uint8_t language_count = Languages.size();
  output->write(reinterpret_cast<char *>(&language_count),
                sizeof(language_count));
  for (int i = 0; i < language_count; ++i) {
    output->write(Languages[i].Code, sizeof(Languages[i].Code));
  }

  return 0;
}

int HelloResp::Read(std::istream *input) {
  char magic[sizeof(Magic)];
  input->read(magic, sizeof(magic));
  int diff = std::strncmp(magic, Magic, sizeof(magic));
  if (diff != 0)
    return -1;

  LanguageCode language = {};
  input->read(reinterpret_cast<char *>(&Language.Code), sizeof(Language.Code));

  return 0;
}

int HelloResp::Write(std::ostream *output) {
  output->write(reinterpret_cast<char *>(Magic), sizeof(Magic));

  output->write(Language.Code, sizeof(Language.Code));

  return 0;
}

int EnumDesc::Read(std::istream *input) {
  uint8_t name_length;
  input->read(reinterpret_cast<char *>(&name_length), sizeof(name_length));
  Name = std::string(name_length, '\0');
  input->read(Name.data(), name_length);

  uint16_t descr_length;
  input->read(reinterpret_cast<char *>(&descr_length), sizeof(descr_length));
  Description = std::string(descr_length, '\0');
  input->read(Description.data(), descr_length);

  uint8_t val_count;
  input->read(reinterpret_cast<char *>(&val_count), sizeof(val_count));
  for (int i = 0; i < val_count; ++i) {
    auto &val = Values.emplace_back();

    uint8_t val_name_length;
    input->read(reinterpret_cast<char *>(&val_name_length),
                sizeof(val_name_length));
    val.Name = std::string(val_name_length, '\0');
    input->read(val.Name.data(), val_name_length);

    uint16_t val_descr_length;
    input->read(reinterpret_cast<char *>(&val_descr_length),
                sizeof(val_descr_length));
    val.Description = std::string(val_descr_length, '\0');
    input->read(val.Description.data(), val_descr_length);
  }

  return 0;
}

int EnumDesc::Write(std::ostream *output) {
  uint8_t name_length = Name.length();
  output->write(reinterpret_cast<char *>(&name_length), sizeof(name_length));
  output->write(Name.c_str(), name_length);

  uint16_t descr_length = Description.length();
  output->write(reinterpret_cast<char *>(&descr_length), sizeof(descr_length));
  output->write(Description.c_str(), descr_length);

  uint8_t value_count = Values.size();
  output->write(reinterpret_cast<char *>(&value_count), sizeof(value_count));
  for (auto &val : Values) {
    uint8_t val_name_length = val.Name.length();
    output->write(reinterpret_cast<char *>(&val_name_length),
                  sizeof(val_name_length));
    output->write(val.Name.c_str(), val_name_length);

    uint16_t val_descr_length = val.Description.length();
    output->write(reinterpret_cast<char *>(&val_descr_length),
                  sizeof(val_descr_length));
    output->write(val.Description.c_str(), val_descr_length);
  }

  return 0;
}

int SignalDesc::Read(std::istream *input) {
  uint8_t name_length;
  input->read(reinterpret_cast<char *>(&name_length), sizeof(name_length));
  Name = std::string(name_length, '\0');
  input->read(Name.data(), name_length);

  uint16_t descr_length;
  input->read(reinterpret_cast<char *>(&descr_length), sizeof(descr_length));
  Description = std::string(descr_length, '\0');
  input->read(Description.data(), descr_length);

  uint8_t arg_count;
  input->read(reinterpret_cast<char *>(&arg_count), sizeof(arg_count));
  for (int i = 0; i < arg_count; ++i) {
    auto &arg = Args.emplace_back();
    input->read(reinterpret_cast<char *>(&arg.Type), sizeof(arg.Type));

    if (arg.Type == TMSG_ENUM)
      input->read(reinterpret_cast<char *>(&arg.Param.EnumId),
                  sizeof(arg.Param.EnumId));

    uint8_t arg_name_length;
    input->read(reinterpret_cast<char *>(&arg_name_length),
                sizeof(arg_name_length));
    arg.Name = std::string(arg_name_length, '\0');
    input->read(arg.Name.data(), arg_name_length);

    uint16_t arg_descr_length;
    input->read(reinterpret_cast<char *>(&arg_descr_length),
                sizeof(arg_descr_length));
    arg.Description = std::string(arg_descr_length, '\0');
    input->read(arg.Description.data(), arg_descr_length);
  }

  return 0;
}

int SignalDesc::Write(std::ostream *output) {
  uint8_t name_length = Name.length();
  output->write(reinterpret_cast<char *>(&name_length), sizeof(name_length));
  output->write(Name.c_str(), name_length);

  uint16_t descr_length = Description.length();
  output->write(reinterpret_cast<char *>(&descr_length), sizeof(descr_length));
  output->write(Description.c_str(), descr_length);

  uint8_t arg_count = Args.size();
  output->write(reinterpret_cast<char *>(&arg_count), sizeof(arg_count));
  for (auto &arg : Args) {
    output->write(reinterpret_cast<char *>(&arg.Type), sizeof(arg.Type));

    if (arg.Type == TMSG_ENUM)
      output->write(reinterpret_cast<char *>(&arg.Param.EnumId),
                    sizeof(arg.Param.EnumId));

    uint8_t arg_name_length = arg.Name.length();
    output->write(reinterpret_cast<char *>(&arg_name_length),
                  sizeof(arg_name_length));
    output->write(arg.Name.c_str(), arg_name_length);

    uint16_t arg_descr_length = arg.Description.length();
    output->write(reinterpret_cast<char *>(&arg_descr_length),
                  sizeof(arg_descr_length));
    output->write(arg.Description.c_str(), arg_descr_length);
  }

  return 0;
}

int SignalList::Read(std::istream *input) {
  uint16_t enum_count;
  input->read(reinterpret_cast<char *>(&enum_count), sizeof(enum_count));
  for (int i = 0; i < enum_count; ++i) {
    auto &num = Enums.emplace_back();
    num.Read(input);
  }

  uint16_t in_signal_count;
  input->read(reinterpret_cast<char *>(&in_signal_count),
              sizeof(in_signal_count));
  for (int i = 0; i < in_signal_count; ++i) {
    auto &signal = In.emplace_back();
    signal.Read(input);
  }

  uint16_t out_signal_count;
  input->read(reinterpret_cast<char *>(&out_signal_count),
              sizeof(out_signal_count));
  for (int i = 0; i < out_signal_count; ++i) {
    auto &signal = Out.emplace_back();
    signal.Read(input);
  }

  return 0;
}

int SignalList::Write(std::ostream *output) {
  uint16_t enum_count = Enums.size();
  output->write(reinterpret_cast<char *>(&enum_count), sizeof(enum_count));
  for (auto &num : Enums)
    num.Write(output);

  uint16_t in_signal_count = In.size();
  output->write(reinterpret_cast<char *>(&in_signal_count),
                sizeof(in_signal_count));
  for (auto &signal : In)
    signal.Write(output);

  uint16_t out_signal_count = Out.size();
  output->write(reinterpret_cast<char *>(&out_signal_count),
                sizeof(out_signal_count));
  for (auto &signal : Out)
    signal.Write(output);

  return 0;
}

int Signal::Read(std::istream *input) {
  input->read(reinterpret_cast<char *>(&Index), sizeof(Index));

  uint8_t arg_count;
  if (IsOut) {
    arg_count = SignalList->Out[Index].Args.size();
  } else {
    arg_count = SignalList->In[Index].Args.size();
  }

  Args.resize(arg_count);
  for (int i = 0; i < arg_count; ++i) {
    auto arg = &Args[i];
    input->read(arg->Chars, sizeof(*arg));
  }

  return 0;
}

int Signal::Write(std::ostream *output) {
  output->write(reinterpret_cast<char *>(&Index), sizeof(Index));

  for (auto &arg : Args)
    output->write(arg.Chars, sizeof(arg));

  return 0;
}
} // namespace Msg
} // namespace Tomato