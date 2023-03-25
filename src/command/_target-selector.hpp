#pragma once

#include <je2be/strings.hpp>

#include "command/_token.hpp"

namespace je2be::command {

struct Argument {
  virtual ~Argument() {}
  virtual void toRawArgument(std::vector<std::pair<std::u8string, std::u8string>> &buffer, Mode mode) const = 0;
};

struct SimpleArgument : public Argument {
  SimpleArgument(std::u8string const &k, std::u8string const &v) : fKey(k), fValue(v) {}

  void toRawArgument(std::vector<std::pair<std::u8string, std::u8string>> &buffer, Mode mode) const override {
    buffer.push_back(std::make_pair(fKey, fValue));
  }

  std::u8string fKey;
  std::u8string fValue;
};

enum class DedicatedArgumentType {
  Limit,
  GameMode,
};

struct DedicatedArgument : public Argument {
  DedicatedArgument(DedicatedArgumentType type, std::u8string const &value) : fType(type), fValue(value) {}

  void toRawArgument(std::vector<std::pair<std::u8string, std::u8string>> &buffer, Mode mode) const override {
    using namespace std;
    u8string name;
    u8string value;
    switch (fType) {
    case DedicatedArgumentType::Limit: {
      value = fValue;
      if (mode == Mode::Bedrock) {
        name = u8"c";
      } else {
        name = u8"limit";
      }
      break;
    }
    case DedicatedArgumentType::GameMode: {
      value = fValue;
      if (mode == Mode::Bedrock) {
        name = u8"m";
      } else {
        name = u8"gamemode";
        unordered_map<u8string, u8string> const mapping = {
            {u8"0", u8"survival"},
            {u8"s", u8"survival"},
            {u8"1", u8"creative"},
            {u8"c", u8"creative"},
            {u8"2", u8"adventure"},
            {u8"a", u8"adventure"},
        };
        auto found = mapping.find(fValue);
        if (found != mapping.end()) {
          value = found->second;
        }
      }
      break;
    }
    default:
      return;
    }
    buffer.push_back(std::make_pair(name, value));
  }

  DedicatedArgumentType fType;
  std::u8string fValue;
};

enum class NumberArgumentType {
  Distance,
  Level,
  XRotation,
  YRotation,
};

struct RangedNumberArgument : public Argument {
  NumberArgumentType const fType;
  std::u8string fMinimum;
  std::u8string fMaximum;

  RangedNumberArgument(NumberArgumentType type, std::u8string const &min, std::u8string const &max) : fType(type), fMinimum(min), fMaximum(max) {
  }

  static std::shared_ptr<RangedNumberArgument> ParseJava(NumberArgumentType type, std::u8string const &value) {
    using namespace std;
    size_t range = value.find(u8"..");
    if (range != string::npos) {
      auto min = value.substr(0, range);
      auto max = value.substr(range + 2);
      return make_shared<RangedNumberArgument>(type, min, max);
    } else {
      return make_shared<RangedNumberArgument>(type, value, value);
    }
  }

  void toRawArgument(std::vector<std::pair<std::u8string, std::u8string>> &buffer, Mode m) const override {
    using namespace std;
    if (m == Mode::Bedrock) {
      u8string min;
      u8string max;
      switch (fType) {
      case NumberArgumentType::Distance: {
        max = u8"r";
        min = u8"rm";
        break;
      }
      case NumberArgumentType::Level: {
        max = u8"l";
        min = u8"lm";
        break;
      }
      case NumberArgumentType::XRotation: {
        max = u8"rx";
        min = u8"rxm";
        break;
      }
      case NumberArgumentType::YRotation: {
        max = u8"ry";
        min = u8"rym";
        break;
      }
      default:
        return;
      }
      if (!fMinimum.empty()) {
        buffer.push_back(make_pair(min, fMinimum));
      }
      if (!fMaximum.empty()) {
        buffer.push_back(make_pair(max, fMaximum));
      }
    } else {
      u8string name;
      switch (fType) {
      case NumberArgumentType::Distance: {
        name = u8"distance";
        break;
      }
      case NumberArgumentType::Level: {
        name = u8"level";
        break;
      }
      case NumberArgumentType::XRotation: {
        name = u8"x_rotation";
        break;
      }
      case NumberArgumentType::YRotation: {
        name = u8"y_rotation";
        break;
      }
      default:
        return;
      }
      if (!fMinimum.empty() && !fMaximum.empty()) {
        if (fMinimum == fMaximum) {
          buffer.push_back(make_pair(name, fMaximum));
        } else {
          buffer.push_back(make_pair(name, fMinimum + u8".." + fMaximum));
        }
      } else if (!fMinimum.empty()) {
        buffer.push_back(make_pair(name, fMinimum + u8".."));
      } else if (!fMaximum.empty()) {
        buffer.push_back(make_pair(name, u8".." + fMaximum));
      }
    }
  }
};

class TargetSelector : public Token {
public:
  TargetSelector(std::u8string const &raw) : Token(raw) {}

  Type type() const override { return Type::TargetSelector; }

  std::u8string toString(Mode m) const override {
    using namespace std;
    if (fArguments.empty()) {
      return fType;
    }
    u8string r = fType + u8"[";
    vector<pair<u8string, u8string>> raw;
    for (auto const &arg : fArguments) {
      arg->toRawArgument(raw, m);
    }
    for (size_t i = 0; i < raw.size(); i++) {
      auto const arg = raw[i];
      r += arg.first + u8"=" + arg.second;
      if (i + 1 < raw.size()) {
        r += u8",";
      }
    }
    return r + u8"]";
  }

  static std::shared_ptr<TargetSelector> Parse(std::u8string const &raw) {
    using namespace std;

    shared_ptr<TargetSelector> ret(new TargetSelector(raw));
    size_t openBracket = raw.find(u8'[');
    if (openBracket == u8string::npos) {
      ret->fType = raw;
      return ret;
    }
    if (!raw.ends_with(u8"]")) {
      return nullptr;
    }
    ret->fType = raw.substr(0, openBracket);
    u8string body = raw.substr(openBracket + 1, raw.length() - openBracket - 2);
    u8string replaced;
    if (!EscapeStringLiteralContents(body, &replaced)) {
      return nullptr;
    }
    size_t pos = 0;
    vector<pair<u8string, u8string>> arguments;
    while (pos < replaced.length()) {
      auto arg = ParseArgument(body, replaced, pos);
      if (!arg) {
        return nullptr;
      }
      arguments.push_back(*arg);
    }
    if (!ParseRawArguments(arguments, ret->fArguments)) {
      return nullptr;
    }
    return ret;
  }

private:
  static void Erase(std::vector<std::pair<std::u8string, std::u8string>> &v, std::u8string const &key) {
    for (int i = 0; i < v.size(); i++) {
      if (v[i].first == key) {
        v.erase(v.begin() + i);
        i--;
      }
    }
  }

  static std::u8string Get(std::vector<std::pair<std::u8string, std::u8string>> const &v, std::u8string const &key) {
    for (int i = 0; i < v.size(); i++) {
      if (v[i].first == key) {
        return v[i].second;
      }
    }
    return u8"";
  }

  static bool ParseRawArguments(std::vector<std::pair<std::u8string, std::u8string>> const &raw, std::vector<std::shared_ptr<Argument>> &parsed) {
    using namespace std;
    vector<pair<u8string, u8string>> args(raw);

    while (!args.empty()) {
      auto pair = args[0];
      u8string k = pair.first;
      u8string v = pair.second;
      if (k == u8"distance") {
        auto distance = RangedNumberArgument::ParseJava(NumberArgumentType::Distance, v);
        if (!distance) {
          return false;
        }
        parsed.push_back(distance);
      } else if (k == u8"r" || k == u8"rm") {
        auto min = Get(args, u8"rm");
        auto max = Get(args, u8"r");
        parsed.push_back(make_shared<RangedNumberArgument>(NumberArgumentType::Distance, min, max));
        Erase(args, u8"r");
        Erase(args, u8"rm");
      } else if (k == u8"level") {
        auto level = RangedNumberArgument::ParseJava(NumberArgumentType::Level, v);
        if (!level) {
          return false;
        }
        parsed.push_back(level);
      } else if (k == u8"l" || k == u8"lm") {
        auto min = Get(args, u8"lm");
        auto max = Get(args, u8"l");
        parsed.push_back(make_shared<RangedNumberArgument>(NumberArgumentType::Level, min, max));
        Erase(args, u8"l");
        Erase(args, u8"lm");
      } else if (k == u8"x_rotation") {
        auto p = RangedNumberArgument::ParseJava(NumberArgumentType::XRotation, v);
        if (!p) {
          return false;
        }
        parsed.push_back(p);
      } else if (k == u8"rx" || k == u8"rxm") {
        auto min = Get(args, u8"rxm");
        auto max = Get(args, u8"rx");
        parsed.push_back(make_shared<RangedNumberArgument>(NumberArgumentType::XRotation, min, max));
        Erase(args, u8"rx");
        Erase(args, u8"rxm");
      } else if (k == u8"y_rotation") {
        auto p = RangedNumberArgument::ParseJava(NumberArgumentType::YRotation, v);
        if (!p) {
          return false;
        }
        parsed.push_back(p);
      } else if (k == u8"ry" || k == u8"rym") {
        auto min = Get(args, u8"rym");
        auto max = Get(args, u8"ry");
        parsed.push_back(make_shared<RangedNumberArgument>(NumberArgumentType::YRotation, min, max));
        Erase(args, u8"ry");
        Erase(args, u8"rym");
      } else if (k == u8"gamemode" || k == u8"m") {
        parsed.push_back(make_shared<DedicatedArgument>(DedicatedArgumentType::GameMode, v));
      } else if (k == u8"limit" || k == u8"c") {
        parsed.push_back(make_shared<DedicatedArgument>(DedicatedArgumentType::Limit, v));
      } else {
        parsed.push_back(make_shared<SimpleArgument>(k, v));
      }
      Erase(args, k);
    }
    return true;
  }

  static std::optional<std::pair<std::u8string, std::u8string>> ParseArgument(std::u8string const &body, std::u8string const &replaced, size_t &pos) {
    using namespace std;

    if (body.substr(pos).starts_with(u8"scores=")) {
      static regex const sScoresRegex(R"((scores=\{[^\}]*\}).*)");
      smatch m;
      u8string s = body.substr(pos);
      string ss;
      ss.assign((char const *)s.data(), s.size());
      if (!std::regex_match(ss, m, sScoresRegex)) {
        return nullopt;
      }
      if (m.size() < 2) {
        return nullopt;
      }
      string kv = m[1].str();
      string value = kv.substr(7);
      pos = pos += kv.size() + 1;
      u8string u8value;
      u8value.assign((char8_t const *)value.data(), value.size());
      return make_pair(u8"scores", u8value);
    } else {
      size_t equal = replaced.find(u8'=', pos);
      if (equal == string::npos) {
        return nullopt;
      }
      size_t comma = replaced.find(',', equal);
      u8string arg = strings::Substring(body, pos, equal);
      u8string value = strings::Substring(body, equal + 1, comma);
      if (comma != string::npos) {
        pos = comma + 1;
      } else {
        pos = comma;
      }
      return make_pair(arg, value);
    }
  }

public:
  std::u8string fType;
  std::vector<std::shared_ptr<Argument>> fArguments;
};

} // namespace je2be::command
