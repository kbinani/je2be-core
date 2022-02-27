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
