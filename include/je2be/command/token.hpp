#pragma once

namespace je2be::command {

enum class Mode {
  Java,
  Bedrock,
};

class Token {
public:
  enum class Type {
    Simple,
    StringLiteral,
    TargetSelector,
    Whitespace,
    Comment,
  };

  Token(std::string const &raw) : fRaw(raw) {}

  virtual Type type() const { return Type::Simple; }

  virtual std::string toString(Mode m) const {
    return fRaw;
  }

  static bool IterateStringLiterals(std::string const &s, std::function<void(std::string const &str, bool stringLiteral)> callback) {
    using namespace std;
    size_t pos = 0;
    if (s.empty()) {
      return true;
    }
    while (pos < s.length()) {
      auto quote = s.find_first_of('"', pos);
      if (quote == string::npos) {
        auto part = s.substr(pos);
        callback(part, false);
        return true;
      } else if (quote == 0 || (quote > 0 && s[quote - 1] != '\\')) {
        static regex const sStringLiteralCloseRegex(R"(([^\\"]|\\.)*")");
        smatch m;
        if (!regex_search(s.begin() + quote + 1, s.end(), m, sStringLiteralCloseRegex)) {
          // Cannot find closing "
          return false;
        }
        if (pos < quote) {
          auto pre = strings::Substring(s, pos, quote);
          callback(pre, false);
        }
        auto part = "\"" + m.str();
        callback(part, true);
        pos = quote + part.length();
      }
    }
    return true;
  }

  static bool IterateStringLiterals_(std::string const &s, std::function<void(std::string const &str, bool stringLiteral)> callback) {
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
    // TODO: should fail if incomplete quotation
    return true;
  }

  // Replace contents of string literals to special character '\t'
  static bool EscapeStringLiteralContents(std::string const &s, std::string *result) {
    using namespace std;
    if (!result) {
      return false;
    }
    result->clear();
    return IterateStringLiterals(s, [&result](string const &str, bool stringLiteral) {
      if (stringLiteral) {
        string s = "\"";
        s += string(str.length() - 2, '\t');
        s += "\"";
        *result += s;
      } else {
        *result += str;
      }
    });
  }

public:
  std::string fRaw;
};

class Whitespace : public Token {
public:
  Whitespace(std::string const &raw) : Token(raw) {}
  Type type() const override { return Type::Whitespace; }
};

class Comment : public Token {
public:
  Comment(std::string const &raw) : Token(raw) {}
  virtual Type type() const { return Type::Comment; }
};

class StringLiteral : public Token {
public:
  StringLiteral(std::string const &raw) : Token(raw) {}
  Type type() const override { return Type::StringLiteral; }
};

} // namespace je2be::command
