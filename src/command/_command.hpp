#pragma once

#include <regex>

#include "command/_target-selector.hpp"
#include "command/_token.hpp"

namespace je2be::command {

class Command {
public:
  static std::u8string TranspileJavaToBedrock(std::u8string const &command) {
    return Transpile(command, Mode::Bedrock, u8':', u8'/');
  }

  static std::u8string TranspileBedrockToJava(std::u8string const &command) {
    return Transpile(command, Mode::Java, u8'/', u8':');
  }

  static bool Parse(std::u8string const &command, std::vector<std::shared_ptr<Token>> &tokens) {
    try {
      return ParseUnsafe(command, tokens);
    } catch (...) {
    }
    return false;
  }

  static std::u8string ToString(std::vector<std::shared_ptr<Token>> const &tokens, Mode m) {
    std::u8string r;
    for (auto const &token : tokens) {
      r += token->toString(m);
    }
    return r;
  }

private:
  static std::u8string Transpile(std::u8string const &command, Mode outputMode, char functionNamespaceSeparatorFrom, char functionNamespaceSeparatorTo) {
    using namespace std;

    vector<shared_ptr<Token>> tokens;
    if (!Parse(command, tokens)) {
      return command;
    }
    for (int i = 0; i + 2 < tokens.size(); i++) {
      if (tokens[i]->type() == Token::Type::Simple && tokens[i + 1]->type() == Token::Type::Whitespace && tokens[i + 2]->type() == Token::Type::Simple) {
        if (tokens[i]->fRaw == u8"function") {
          u8string token = tokens[i + 2]->fRaw;
          auto found = token.find(functionNamespaceSeparatorFrom);
          if (found != u8string::npos) {
            token = token.substr(0, found) + u8string(1, functionNamespaceSeparatorTo) + token.substr(found + 1);
            tokens[i + 2]->fRaw = token;
          }
        }
      }
    }

    return ToString(tokens, outputMode);
  }

  static bool ParseUnsafe(std::u8string const &raw, std::vector<std::shared_ptr<Token>> &tokens) {
    using namespace std;

    if (raw.find(u8'\t') != u8string::npos) {
      return false;
    }

    u8string command = raw;
    {
      u8string prefix;
      while (true) {
        if (command.starts_with(u8" ")) {
          prefix += u8" ";
          command = command.substr(1);
        } else if (command.starts_with(u8"/")) {
          if (!prefix.empty()) {
            tokens.push_back(make_shared<Whitespace>(prefix));
          }
          tokens.push_back(make_shared<Token>(u8"/"));
          prefix = u8"";
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

    u8string suffix;
    while (true) {
      if (command.ends_with(u8"\x0d")) {
        suffix = u8"\x0d" + suffix;
        command = command.substr(0, command.size() - 1);
      } else if (command.ends_with(u8" ")) {
        suffix = u8" " + suffix;
        command = command.substr(0, command.size() - 1);
      } else {
        break;
      }
    }

    shared_ptr<Token> suffixToken;

    u8string replaced;
    if (!Token::EscapeStringLiteralContents(command, &replaced)) {
      return false;
    }
    auto comment = replaced.find(u8'#');
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
    string sreplaced;
    sreplaced.assign((char const *)replaced.data(), replaced.size());
    auto begin = sregex_iterator(sreplaced.begin(), sreplaced.end(), sWhitespaceRegex);
    auto end = sregex_iterator();
    ptrdiff_t pos = 0;
    for (auto it = begin; it != end; it++) {
      smatch m = *it;
      auto offset = m.position();
      auto length = m.length();
      if (pos < offset) {
        u8string tokenString = strings::Substring(command, pos, offset);
        u8string tokenStringReplaced = strings::Substring(replaced, pos, offset);
        if (!ParseToken(tokenString, tokenStringReplaced, tokens)) {
          return false;
        }
      }
      string match = m.str();
      u8string u8match;
      u8match.assign((char8_t const *)match.data(), match.size());
      tokens.push_back(make_shared<Whitespace>(u8match));
      pos = offset + length;
    }
    if (pos < command.length()) {
      u8string tokenString = strings::Substring(command, pos);
      u8string tokenStringReplaced = strings::Substring(replaced, pos);
      if (!ParseToken(tokenString, tokenStringReplaced, tokens)) {
        return false;
      }
    }

    if (suffixToken) {
      tokens.push_back(suffixToken);
    }
    return true;
  }

  static bool ParseToken(std::u8string const &command, std::u8string const &replaced, std::vector<std::shared_ptr<Token>> &tokens) {
    using namespace std;
    static regex const sTargetSelectorRegex(R"(@(p|r|a|e|s|c|v|initiator)(\[[^\]]*\])?)");
    ptrdiff_t pos = 0;
    string sreplaced;
    sreplaced.assign((char const *)replaced.data(), replaced.size());
    auto begin = sregex_iterator(sreplaced.begin(), sreplaced.end(), sTargetSelectorRegex);
    auto end = sregex_iterator();
    for (auto it = begin; it != end; it++) {
      smatch m = *it;
      auto offset = m.position();
      auto length = m.length();
      if (pos < offset) {
        u8string str = command.substr(pos, offset - pos);
        Token::IterateStringLiterals(str, [&tokens](u8string const &s, bool stringLiteral) {
          if (stringLiteral) {
            tokens.push_back(make_shared<StringLiteral>(s));
          } else {
            tokens.push_back(make_shared<Token>(s));
          }
        });
      }
      auto ts = TargetSelector::Parse(command.substr(offset, length));
      if (!ts) {
        return false;
      }
      tokens.push_back(ts);
      pos = offset + length;
    }
    if (pos < command.length()) {
      u8string str = command.substr(pos);
      Token::IterateStringLiterals(str, [&tokens](u8string const &s, bool stringLiteral) {
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

} // namespace je2be::command
