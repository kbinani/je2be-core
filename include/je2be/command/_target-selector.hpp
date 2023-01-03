#pragma once

#include <je2be/strings.hpp>

#include <je2be/command/_token.hpp>

namespace je2be::command {

struct Argument {
  virtual ~Argument() {}
  virtual void toRawArgument(std::vector<std::pair<std::string, std::string>> &buffer, Mode mode) const = 0;
};

struct SimpleArgument : public Argument {
  SimpleArgument(std::string const &k, std::string const &v) : fKey(k), fValue(v) {}

  void toRawArgument(std::vector<std::pair<std::string, std::string>> &buffer, Mode mode) const override {
    buffer.push_back(std::make_pair(fKey, fValue));
  }

  std::string fKey;
  std::string fValue;
};

enum class DedicatedArgumentType {
  Limit,
  GameMode,
};

struct DedicatedArgument : public Argument {
  DedicatedArgument(DedicatedArgumentType type, std::string const &value) : fType(type), fValue(value) {}

  void toRawArgument(std::vector<std::pair<std::string, std::string>> &buffer, Mode mode) const override {
    using namespace std;
    string name;
    string value;
    switch (fType) {
    case DedicatedArgumentType::Limit: {
      value = fValue;
      if (mode == Mode::Bedrock) {
        name = "c";
      } else {
        name = "limit";
      }
      break;
    }
    case DedicatedArgumentType::GameMode: {
      value = fValue;
      if (mode == Mode::Bedrock) {
        name = "m";
      } else {
        name = "gamemode";
        unordered_map<string, string> const mapping = {
            {"0", "survival"},
            {"s", "survival"},
            {"1", "creative"},
            {"c", "creative"},
            {"2", "adventure"},
            {"a", "adventure"},
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
  std::string fValue;
};

enum class NumberArgumentType {
  Distance,
  Level,
  XRotation,
  YRotation,
};

struct RangedNumberArgument : public Argument {
  NumberArgumentType const fType;
  std::string fMinimum;
  std::string fMaximum;

  RangedNumberArgument(NumberArgumentType type, std::string const &min, std::string const &max) : fType(type), fMinimum(min), fMaximum(max) {
  }

  static std::shared_ptr<RangedNumberArgument> ParseJava(NumberArgumentType type, std::string const &value) {
    using namespace std;
    size_t range = value.find("..");
    if (range != string::npos) {
      auto min = value.substr(0, range);
      auto max = value.substr(range + 2);
      return make_shared<RangedNumberArgument>(type, min, max);
    } else {
      return make_shared<RangedNumberArgument>(type, value, value);
    }
  }

  void toRawArgument(std::vector<std::pair<std::string, std::string>> &buffer, Mode m) const override {
    using namespace std;
    if (m == Mode::Bedrock) {
      string min;
      string max;
      switch (fType) {
      case NumberArgumentType::Distance: {
        max = "r";
        min = "rm";
        break;
      }
      case NumberArgumentType::Level: {
        max = "l";
        min = "lm";
        break;
      }
      case NumberArgumentType::XRotation: {
        max = "rx";
        min = "rxm";
        break;
      }
      case NumberArgumentType::YRotation: {
        max = "ry";
        min = "rym";
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
      string name;
      switch (fType) {
      case NumberArgumentType::Distance: {
        name = "distance";
        break;
      }
      case NumberArgumentType::Level: {
        name = "level";
        break;
      }
      case NumberArgumentType::XRotation: {
        name = "x_rotation";
        break;
      }
      case NumberArgumentType::YRotation: {
        name = "y_rotation";
        break;
      }
      default:
        return;
      }
      if (!fMinimum.empty() && !fMaximum.empty()) {
        if (fMinimum == fMaximum) {
          buffer.push_back(make_pair(name, fMaximum));
        } else {
          buffer.push_back(make_pair(name, fMinimum + ".." + fMaximum));
        }
      } else if (!fMinimum.empty()) {
        buffer.push_back(make_pair(name, fMinimum + ".."));
      } else if (!fMaximum.empty()) {
        buffer.push_back(make_pair(name, ".." + fMaximum));
      }
    }
  }
};

class TargetSelector : public Token {
public:
  TargetSelector(std::string const &raw) : Token(raw) {}

  Type type() const override { return Type::TargetSelector; }

  std::string toString(Mode m) const override {
    using namespace std;
    if (fArguments.empty()) {
      return fType;
    }
    string r = fType + "[";
    vector<pair<string, string>> raw;
    for (auto const &arg : fArguments) {
      arg->toRawArgument(raw, m);
    }
    for (size_t i = 0; i < raw.size(); i++) {
      auto const arg = raw[i];
      r += arg.first + "=" + arg.second;
      if (i + 1 < raw.size()) {
        r += ",";
      }
    }
    return r + "]";
  }

  static std::shared_ptr<TargetSelector> Parse(std::string const &raw) {
    using namespace std;

    shared_ptr<TargetSelector> ret(new TargetSelector(raw));
    size_t openBracket = raw.find('[');
    if (openBracket == string::npos) {
      ret->fType = raw;
      return ret;
    }
    if (!raw.ends_with("]")) {
      return nullptr;
    }
    ret->fType = raw.substr(0, openBracket);
    string body = raw.substr(openBracket + 1, raw.length() - openBracket - 2);
    string replaced;
    if (!EscapeStringLiteralContents(body, &replaced)) {
      return nullptr;
    }
    size_t pos = 0;
    vector<pair<string, string>> arguments;
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
  static void Erase(std::vector<std::pair<std::string, std::string>> &v, std::string const &key) {
    for (int i = 0; i < v.size(); i++) {
      if (v[i].first == key) {
        v.erase(v.begin() + i);
        i--;
      }
    }
  }

  static std::string Get(std::vector<std::pair<std::string, std::string>> const &v, std::string const &key) {
    for (int i = 0; i < v.size(); i++) {
      if (v[i].first == key) {
        return v[i].second;
      }
    }
    return "";
  }

  static bool ParseRawArguments(std::vector<std::pair<std::string, std::string>> const &raw, std::vector<std::shared_ptr<Argument>> &parsed) {
    using namespace std;
    vector<pair<string, string>> args(raw);

    while (!args.empty()) {
      auto pair = args[0];
      string k = pair.first;
      string v = pair.second;
      if (k == "distance") {
        auto distance = RangedNumberArgument::ParseJava(NumberArgumentType::Distance, v);
        if (!distance) {
          return false;
        }
        parsed.push_back(distance);
      } else if (k == "r" || k == "rm") {
        auto min = Get(args, "rm");
        auto max = Get(args, "r");
        parsed.push_back(make_shared<RangedNumberArgument>(NumberArgumentType::Distance, min, max));
        Erase(args, "r");
        Erase(args, "rm");
      } else if (k == "level") {
        auto level = RangedNumberArgument::ParseJava(NumberArgumentType::Level, v);
        if (!level) {
          return false;
        }
        parsed.push_back(level);
      } else if (k == "l" || k == "lm") {
        auto min = Get(args, "lm");
        auto max = Get(args, "l");
        parsed.push_back(make_shared<RangedNumberArgument>(NumberArgumentType::Level, min, max));
        Erase(args, "l");
        Erase(args, "lm");
      } else if (k == "x_rotation") {
        auto p = RangedNumberArgument::ParseJava(NumberArgumentType::XRotation, v);
        if (!p) {
          return false;
        }
        parsed.push_back(p);
      } else if (k == "rx" || k == "rxm") {
        auto min = Get(args, "rxm");
        auto max = Get(args, "rx");
        parsed.push_back(make_shared<RangedNumberArgument>(NumberArgumentType::XRotation, min, max));
        Erase(args, "rx");
        Erase(args, "rxm");
      } else if (k == "y_rotation") {
        auto p = RangedNumberArgument::ParseJava(NumberArgumentType::YRotation, v);
        if (!p) {
          return false;
        }
        parsed.push_back(p);
      } else if (k == "ry" || k == "rym") {
        auto min = Get(args, "rym");
        auto max = Get(args, "ry");
        parsed.push_back(make_shared<RangedNumberArgument>(NumberArgumentType::YRotation, min, max));
        Erase(args, "ry");
        Erase(args, "rym");
      } else if (k == "gamemode" || k == "m") {
        parsed.push_back(make_shared<DedicatedArgument>(DedicatedArgumentType::GameMode, v));
      } else if (k == "limit" || k == "c") {
        parsed.push_back(make_shared<DedicatedArgument>(DedicatedArgumentType::Limit, v));
      } else {
        parsed.push_back(make_shared<SimpleArgument>(k, v));
      }
      Erase(args, k);
    }
    return true;
  }

  static std::optional<std::pair<std::string, std::string>> ParseArgument(std::string const &body, std::string const &replaced, size_t &pos) {
    using namespace std;
    if (body.substr(pos).starts_with("scores=")) {
      static regex const sScoresRegex(R"((scores=\{[^\}]*\}).*)");
      smatch m;
      string s = body.substr(pos);
      if (!std::regex_match(s, m, sScoresRegex)) {
        return nullopt;
      }
      if (m.size() < 2) {
        return nullopt;
      }
      string kv = m[1].str();
      string value = kv.substr(7);
      pos = pos += kv.size() + 1;
      return make_pair("scores", value);
    } else {
      size_t equal = replaced.find('=', pos);
      if (equal == string::npos) {
        return nullopt;
      }
      size_t comma = replaced.find(',', equal);
      string arg = strings::Substring(body, pos, equal);
      string value = strings::Substring(body, equal + 1, comma);
      if (comma != string::npos) {
        pos = comma + 1;
      } else {
        pos = comma;
      }
      return make_pair(arg, value);
    }
  }

public:
  std::string fType;
  std::vector<std::shared_ptr<Argument>> fArguments;
};

} // namespace je2be::command
