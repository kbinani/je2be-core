#pragma once

namespace je2be::command {

struct Argument {
  ~Argument() {}
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
  std::optional<float> fMinimum;
  std::optional<float> fMaximum;

  RangedNumberArgument(NumberArgumentType type, std::optional<float> min, std::optional<float> max) : fType(type), fMinimum(min), fMaximum(max) {
  }

  static std::shared_ptr<RangedNumberArgument> ParseJava(NumberArgumentType type, std::string const &value) {
    using namespace std;
    size_t range = value.find("..");
    if (range != string::npos) {
      auto first = value.substr(0, range);
      auto second = value.substr(range + 2);

      auto iMin = strings::Toi(first);
      auto fMin = strings::Tof(first);
      auto fMax = strings::Tof(second);

      if (iMin && to_string(*iMin) == first) {
        fMin = *iMin - 1;
      }
      return make_shared<RangedNumberArgument>(type, fMin, fMax);
    } else {
      auto vi = strings::Toi(value);
      if (vi && to_string(*vi) == value) {
        return make_shared<RangedNumberArgument>(type, *vi - 1, *vi);
      }
      auto vf = strings::Tof(value);
      if (vf) {
        return make_shared<RangedNumberArgument>(type, *vf, *vf);
      }
    }
    return nullptr;
  }

  static std::string ToString(float v) {
    if (v == roundf(v)) {
      return std::to_string((int)v);
    } else {
      std::ostringstream ss;
      ss << v;
      return ss.str();
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
      if (fMinimum) {
        buffer.push_back(make_pair(min, ToString(*fMinimum)));
      }
      if (fMaximum) {
        buffer.push_back(make_pair(max, ToString(*fMaximum)));
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
      if (fMinimum && fMaximum) {
        if (*fMinimum + 1 == fMaximum) {
          buffer.push_back(make_pair(name, ToString(*fMaximum)));
        } else {
          buffer.push_back(make_pair(name, ToString(*fMinimum) + ".." + ToString(*fMaximum)));
        }
      } else if (fMinimum) {
        buffer.push_back(make_pair(name, ToString(*fMinimum) + ".."));
      } else if (fMaximum) {
        buffer.push_back(make_pair(name, ".." + ToString(*fMaximum)));
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
    string replaced = EscapeStringLiteralContents(body);
    size_t pos = 0;
    vector<pair<string, string>> arguments;
    while (pos < replaced.length()) {
      auto arg = ParseArgument(body, replaced, pos);
      if (!arg) {
        return nullptr;
      }
      arguments.push_back(*arg);
    }
    ParseRawArguments(arguments, ret->fArguments);
    return ret;
  }

private:
  static void ParseRawArguments(std::vector<std::pair<std::string, std::string>> const &raw, std::vector<std::shared_ptr<Argument>> &parsed) {
    using namespace std;
    unordered_map<string, string> map(raw.begin(), raw.end());

    while (!map.empty()) {
      auto pair = *map.begin();
      string k = pair.first;
      string v = pair.second;
      if (k == "distance") {
        if (auto distance = RangedNumberArgument::ParseJava(NumberArgumentType::Distance, v); distance) {
          parsed.push_back(distance);
        }
      } else if (k == "r" || k == "rm") {
        auto max = strings::Tof(map["r"]);
        auto min = strings::Tof(map["rm"]);
        parsed.push_back(make_shared<RangedNumberArgument>(NumberArgumentType::Distance, min, max));
        map.erase("r");
        map.erase("rm");
      } else if (k == "level") {
        if (auto level = RangedNumberArgument::ParseJava(NumberArgumentType::Level, v); level) {
          parsed.push_back(level);
        }
      } else if (k == "l" || k == "lm") {
        auto max = strings::Tof(map["l"]);
        auto min = strings::Tof(map["lm"]);
        parsed.push_back(make_shared<RangedNumberArgument>(NumberArgumentType::Level, min, max));
        map.erase("l");
        map.erase("lm");
      } else if (k == "x_rotation") {
        if (auto p = RangedNumberArgument::ParseJava(NumberArgumentType::XRotation, v); p) {
          parsed.push_back(p);
        }
      } else if (k == "rx" || k == "rxm") {
        auto max = strings::Tof(map["rx"]);
        auto min = strings::Tof(map["rxm"]);
        parsed.push_back(make_shared<RangedNumberArgument>(NumberArgumentType::XRotation, min, max));
        map.erase("rx");
        map.erase("rxm");
      } else if (k == "y_rotation") {
        if (auto p = RangedNumberArgument::ParseJava(NumberArgumentType::YRotation, v); p) {
          parsed.push_back(p);
        }
      } else if (k == "ry" || k == "rym") {
        auto max = strings::Tof(map["ry"]);
        auto min = strings::Tof(map["rym"]);
        parsed.push_back(make_shared<RangedNumberArgument>(NumberArgumentType::YRotation, min, max));
        map.erase("ry");
        map.erase("rym");
      } else if (k == "gamemode" || k == "m") {
        parsed.push_back(make_shared<DedicatedArgument>(DedicatedArgumentType::GameMode, v));
      } else if (k == "limit" || k == "c") {
        parsed.push_back(make_shared<DedicatedArgument>(DedicatedArgumentType::Limit, v));
      } else {
        parsed.push_back(make_shared<SimpleArgument>(k, v));
      }
      map.erase(k);
    }
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
