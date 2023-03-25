#pragma once

#include <regex>

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

  Token(std::u8string const &raw) : fRaw(raw) {}
  virtual ~Token() {}

  virtual Type type() const { return Type::Simple; }

  virtual std::u8string toString(Mode m) const {
    return fRaw;
  }

  static bool IterateStringLiterals(std::u8string const &s, std::function<void(std::u8string const &str, bool stringLiteral)> callback) {
    using namespace std;
    static regex const sStringLiteralRegex(R"((\"(\\.|[^"\\])*\"?|'(\\.|[^'\\])*'?))");

    ptrdiff_t pos = 0;
    string ns;
    ns.assign((char const *)s.data(), s.size());
    auto begin = sregex_iterator(ns.begin(), ns.end(), sStringLiteralRegex);
    auto end = sregex_iterator();
    for (auto it = begin; it != end; it++) {
      smatch m = *it;
      auto offset = m.position();
      auto length = m.length();
      if (pos < offset) {
        auto pre = s.substr(pos, offset - pos);
        callback(pre, false);
      }
      auto part = m.str();
      u8string u8part;
      u8part.assign((char8_t const *)part.data(), part.size());
      callback(u8part, true);
      if (!part.ends_with(part.at(0))) {
        // Wasn't enclosed with correct quoatation
        return false;
      }
      pos = offset + length;
    }
    if (pos < s.length()) {
      callback(s.substr(pos), false);
    }
    return true;
  }

  // Replace contents of string literals to special character '\t'
  static bool EscapeStringLiteralContents(std::u8string const &s, std::u8string *result) {
    using namespace std;
    if (!result) {
      return false;
    }
    result->clear();
    return IterateStringLiterals(s, [&result](u8string const &str, bool stringLiteral) {
      if (stringLiteral) {
        u8string s = u8"\"";
        s += u8string(str.length() - 2, u8'\t');
        s += u8"\"";
        *result += s;
      } else {
        *result += str;
      }
    });
  }

public:
  std::u8string fRaw;
};

class Whitespace : public Token {
public:
  Whitespace(std::u8string const &raw) : Token(raw) {}
  Type type() const override { return Type::Whitespace; }
};

class Comment : public Token {
public:
  Comment(std::u8string const &raw) : Token(raw) {}
  virtual Type type() const override { return Type::Comment; }
};

class StringLiteral : public Token {
public:
  StringLiteral(std::u8string const &raw) : Token(raw) {}
  Type type() const override { return Type::StringLiteral; }
};

} // namespace je2be::command
