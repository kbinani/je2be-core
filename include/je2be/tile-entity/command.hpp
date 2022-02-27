#pragma once

namespace je2be {

class Command {
public:
  static std::string TranspileJavaToBedrock(std::string const &inLine) {
    using namespace std;

    vector<shared_ptr<Token>> tokens;
    if (!Parse(inLine, tokens, Mode::Java)) {
      return inLine;
    }
    for (int i = 0; i + 2 < tokens.size(); i++) {
      if (tokens[i]->type() == Type::Simple && tokens[i + 1]->type() == Type::Whitespace && tokens[i + 2]->type() == Type::Simple) {
        if (tokens[i]->fRaw == "function") {
          string token = tokens[i + 2]->fRaw;
          auto found = token.find(':');
          if (found != string::npos) {
            token = token.substr(0, found) + "/" + token.substr(found + 1);
            tokens[i + 2]->fRaw = token;
          }
        }
      }
    }

    return ToString(tokens, Mode::Bedrock);
  }

  enum class Mode {
    Java,
    Bedrock,
  };

  enum class Type {
    Simple,
    StringLiteral,
    TargetSelector,
    Whitespace,
    Comment,
  };

  class Token {
  public:
    Token(std::string const &raw, Mode const mode) : fRaw(raw), fMode(mode) {}

    virtual Type type() const { return Type::Simple; }

    virtual std::string toString(Mode m) const {
      return fRaw;
    }

  public:
    std::string fRaw;
    Mode const fMode;
  };

  class Whitespace : public Token {
  public:
    Whitespace(std::string const &raw, Mode m) : Token(raw, m) {}
    Type type() const override { return Type::Whitespace; }
  };

  class Comment : public Token {
  public:
    Comment(std::string const &raw, Mode m) : Token(raw, m) {}
    virtual Type type() const { return Type::Comment; }
  };

  class StringLiteral : public Token {
  public:
    StringLiteral(std::string const &raw, Mode m) : Token(raw, m) {}
    Type type() const override { return Type::StringLiteral; }
  };

  class TargetSelector : public Token {
  public:
    TargetSelector(std::string const &raw, Mode m) : Token(raw, m) {}
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

    static std::shared_ptr<TargetSelector> Parse(std::string const &raw, Mode m) {
      using namespace std;

      shared_ptr<TargetSelector> ret(new TargetSelector(raw, m));
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

      virtual void toRawArgument(std::vector<std::pair<std::string, std::string>> &buffer, Mode m) const override {
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
          map.erase(k);
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
          map.erase(k);
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
          map.erase(k);
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
          map.erase(k);
        } else if (k == "ry" || k == "rym") {
          auto max = strings::Tof(map["ry"]);
          auto min = strings::Tof(map["rym"]);
          parsed.push_back(make_shared<RangedNumberArgument>(NumberArgumentType::YRotation, min, max));
          map.erase("ry");
          map.erase("rym");
        } else if (k == "gamemode" || k == "m") {
          parsed.push_back(make_shared<DedicatedArgument>(DedicatedArgumentType::GameMode, v));
          map.erase(k);
        } else if (k == "limit" || k == "c") {
          parsed.push_back(make_shared<DedicatedArgument>(DedicatedArgumentType::Limit, v));
        } else {
          parsed.push_back(make_shared<SimpleArgument>(k, v));
          map.erase(k);
        }
      }
    }

  private:
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

  static bool Parse(std::string const &command, std::vector<std::shared_ptr<Token>> &tokens, Mode m) {
    try {
      return ParseUnsafe(command, tokens, m);
    } catch (...) {
    }
    return false;
  }

  static std::string ToString(std::vector<std::shared_ptr<Token>> const &tokens, Mode m) {
    std::string r;
    for (auto const &token : tokens) {
      r += token->toString(m);
    }
    return r;
  }

private:
  static void IterateStringLiterals(std::string const &s, std::function<void(std::string const &str, bool stringLiteral)> callback) {
    using namespace std;
    static regex const sStringLiteralRegex(R"("([^\\"]|\\.)*")");

    ptrdiff_t pos = 0;
    auto begin = sregex_iterator(s.begin(), s.end(), sStringLiteralRegex);
    auto end = sregex_iterator();
    for (auto it = begin; it != end; it++) {
      smatch m = *it;
      auto offset = m.position();
      auto length = m.length();
      if (pos < offset) {
        auto part = s.substr(pos, offset - pos);
        callback(part, false);
      }
      callback(m.str(), true);
      pos = offset + length;
    }
    if (pos < s.length()) {
      callback(s.substr(pos), false);
    }
  }

  // Replace contents of string literals to special character '\t'
  static std::string EscapeStringLiteralContents(std::string const &s) {
    using namespace std;
    string replaced;
    IterateStringLiterals(s, [&replaced](string const &str, bool stringLiteral) {
      if (stringLiteral) {
        string s = "\"";
        s += string(str.length() - 2, '\t');
        s += "\"";
        replaced += s;
      } else {
        replaced += str;
      }
    });
    return replaced;
  }

  static bool ParseUnsafe(std::string const &raw, std::vector<std::shared_ptr<Token>> &tokens, Mode mode) {
    using namespace std;

    if (raw.find('\t') != string::npos) {
      return false;
    }

    string command = raw;
    {
      string prefix;
      while (true) {
        if (command.starts_with(" ")) {
          prefix += " ";
          command = command.substr(1);
        } else if (command.starts_with("/")) {
          if (!prefix.empty()) {
            tokens.push_back(make_shared<Whitespace>(prefix, mode));
          }
          tokens.push_back(make_shared<Token>("/", mode));
          prefix = "";
          command = command.substr(1);
          break;
        } else {
          break;
        }
      }
      if (!prefix.empty()) {
        tokens.push_back(make_shared<Whitespace>(prefix, mode));
      }
    }

    string suffix;
    while (true) {
      if (command.ends_with("\x0d")) {
        suffix = "\x0d" + suffix;
        command = command.substr(0, command.size() - 1);
      } else if (command.ends_with(" ")) {
        suffix = " " + suffix;
        command = command.substr(0, command.size() - 1);
      } else {
        break;
      }
    }

    shared_ptr<Token> suffixToken;

    auto replaced = EscapeStringLiteralContents(command);
    auto comment = replaced.find('#');
    if (comment == string::npos) {
      if (!suffix.empty()) {
        suffixToken = make_shared<Whitespace>(suffix, mode);
      }
    } else {
      auto commentString = command.substr(comment);
      suffixToken = make_shared<Comment>(commentString + suffix, mode);
      command = command.substr(0, comment);
      replaced = replaced.substr(0, comment);
    }

    static regex const sWhitespaceRegex(R"([ ]+)");
    auto begin = sregex_iterator(replaced.begin(), replaced.end(), sWhitespaceRegex);
    auto end = sregex_iterator();
    ptrdiff_t pos = 0;
    for (auto it = begin; it != end; it++) {
      smatch m = *it;
      auto offset = m.position();
      auto length = m.length();
      if (pos < offset) {
        string tokenString = strings::Substring(command, pos, offset);
        string tokenStringReplaced = strings::Substring(replaced, pos, offset);
        if (!ParseToken(tokenString, tokenStringReplaced, tokens, mode)) {
          return false;
        }
      }
      tokens.push_back(make_shared<Whitespace>(m.str(), mode));
      pos = offset + length;
    }
    if (pos < command.length()) {
      string tokenString = strings::Substring(command, pos);
      string tokenStringReplaced = strings::Substring(replaced, pos);
      if (!ParseToken(tokenString, tokenStringReplaced, tokens, mode)) {
        return false;
      }
    }

    if (suffixToken) {
      tokens.push_back(suffixToken);
    }
    return true;
  }

  static bool ParseToken(std::string const &command, std::string const &replaced, std::vector<std::shared_ptr<Token>> &tokens, Mode mode) {
    using namespace std;
    static regex const sTargetSelectorRegex(R"(@(p|r|a|e|s|c|v|initiator)(\[[^\]]*\])?)");
    ptrdiff_t pos = 0;
    auto begin = sregex_iterator(replaced.begin(), replaced.end(), sTargetSelectorRegex);
    auto end = sregex_iterator();
    for (auto it = begin; it != end; it++) {
      smatch m = *it;
      auto offset = m.position();
      auto length = m.length();
      if (pos < offset) {
        string str = command.substr(pos, offset - pos);
        IterateStringLiterals(str, [&tokens, mode](string const &s, bool stringLiteral) {
          if (stringLiteral) {
            tokens.push_back(make_shared<StringLiteral>(s, mode));
          } else {
            tokens.push_back(make_shared<Token>(s, mode));
          }
        });
      }
      auto ts = TargetSelector::Parse(m.str(), mode);
      if (!ts) {
        return false;
      }
      tokens.push_back(ts);
      pos = offset + length;
    }
    if (pos < command.length()) {
      string str = command.substr(pos);
      IterateStringLiterals(str, [&tokens, mode](string const &s, bool stringLiteral) {
        if (stringLiteral) {
          tokens.push_back(make_shared<StringLiteral>(s, mode));
        } else {
          tokens.push_back(make_shared<Token>(s, mode));
        }
      });
    }
    return true;
  }

  Command() = delete;
};

} // namespace je2be
