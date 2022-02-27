#pragma once

namespace je2be {

class Command {
public:
  static std::string TranspileJavaToBedrock(std::string const &inLine) {
    using namespace std;

    vector<shared_ptr<Token>> tokens;
    if (!Parse(inLine, tokens)) {
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
    explicit Token(std::string const &raw) : fRaw(raw) {}

    virtual Type type() const { return Type::Simple; }

    virtual std::string toString(Mode m) const {
      return fRaw;
    }

  public:
    std::string fRaw;
  };

  class Whitespace : public Token {
  public:
    explicit Whitespace(std::string const &raw) : Token(raw) {}
    Type type() const override { return Type::Whitespace; }
  };

  class Comment : public Token {
  public:
    explicit Comment(std::string const &raw) : Token(raw) {}
    virtual Type type() const { return Type::Comment; }
  };

  class StringLiteral : public Token {
  public:
    explicit StringLiteral(std::string const &raw) : Token(raw) {}
    Type type() const override { return Type::StringLiteral; }
  };

  class TargetSelector : public Token {
    explicit TargetSelector(std::string const &raw) : Token(raw) {}
    Type type() const override { return Type::TargetSelector; }

  public:
    std::string toString(Mode m) const override {
      if (fArguments.empty()) {
        return fType;
      }
      std::string r = fType + "[";
      for (size_t i = 0; i < fArguments.size(); i++) {
        auto const &arg = fArguments[i];
        r += arg.first + "=" + arg.second;
        if (i + 1 < fArguments.size()) {
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
      while (pos < replaced.length()) {
        auto arg = ParseArgument(body, replaced, pos);
        if (!arg) {
          return nullptr;
        }
        ret->fArguments.push_back(*arg);
      }
      return ret;
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
    std::vector<std::pair<std::string, std::string>> fArguments;
  };

  static bool Parse(std::string const &command, std::vector<std::shared_ptr<Token>> &tokens) {
    try {
      return ParseUnsafe(command, tokens);
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

  static bool ParseUnsafe(std::string const &raw, std::vector<std::shared_ptr<Token>> &tokens) {
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
            tokens.push_back(make_shared<Whitespace>(prefix));
          }
          tokens.push_back(make_shared<Token>("/"));
          prefix = "";
          command = command.substr(1);
          break;
        } else {
          break;
        }
      }
      if (!prefix.empty()) {
        tokens.push_back(make_shared<Whitespace>(prefix));
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
        suffixToken = make_shared<Whitespace>(suffix);
      }
    } else {
      auto commentString = command.substr(comment);
      suffixToken = make_shared<Comment>(commentString + suffix);
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
        if (!ParseToken(tokenString, tokenStringReplaced, tokens)) {
          return false;
        }
      }
      tokens.push_back(make_shared<Whitespace>(m.str()));
      pos = offset + length;
    }
    if (pos < command.length()) {
      string tokenString = strings::Substring(command, pos);
      string tokenStringReplaced = strings::Substring(replaced, pos);
      if (!ParseToken(tokenString, tokenStringReplaced, tokens)) {
        return false;
      }
    }

    if (suffixToken) {
      tokens.push_back(suffixToken);
    }
    return true;
  }

  static bool ParseToken(std::string const &command, std::string const &replaced, std::vector<std::shared_ptr<Token>> &tokens) {
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
        IterateStringLiterals(str, [&tokens](string const &s, bool stringLiteral) {
          if (stringLiteral) {
            tokens.push_back(make_shared<StringLiteral>(s));
          } else {
            tokens.push_back(make_shared<Token>(s));
          }
        });
      }
      auto ts = TargetSelector::Parse(m.str());
      if (!ts) {
        return false;
      }
      tokens.push_back(ts);
      pos = offset + length;
    }
    if (pos < command.length()) {
      string str = command.substr(pos);
      IterateStringLiterals(str, [&tokens](string const &s, bool stringLiteral) {
        if (stringLiteral) {
          tokens.push_back(make_shared<StringLiteral>(s));
        } else {
          tokens.push_back(make_shared<Token>(s));
        }
      });
    }
    return true;
  }

  Command() = delete;
};

} // namespace je2be
