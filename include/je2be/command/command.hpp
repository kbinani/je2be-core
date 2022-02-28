#pragma once

namespace je2be::command {

class Command {
public:
  static std::string TranspileJavaToBedrock(std::string const &command) {
    return Transpile(command, Mode::Bedrock, ':', '/');
  }

  static std::string TranspileBedrockToJava(std::string const &command) {
    return Transpile(command, Mode::Java, '/', ':');
  }

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
  static std::string Transpile(std::string const &command, Mode outputMode, char functionNamespaceSeparatorFrom, char functionNamespaceSeparatorTo) {
    using namespace std;

    vector<shared_ptr<Token>> tokens;
    if (!Parse(command, tokens)) {
      return command;
    }
    for (int i = 0; i + 2 < tokens.size(); i++) {
      if (tokens[i]->type() == Token::Type::Simple && tokens[i + 1]->type() == Token::Type::Whitespace && tokens[i + 2]->type() == Token::Type::Simple) {
        if (tokens[i]->fRaw == "function") {
          string token = tokens[i + 2]->fRaw;
          auto found = token.find(functionNamespaceSeparatorFrom);
          if (found != string::npos) {
            token = token.substr(0, found) + string(1, functionNamespaceSeparatorTo) + token.substr(found + 1);
            tokens[i + 2]->fRaw = token;
          }
        }
      }
    }

    return ToString(tokens, outputMode);
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

    auto replaced = Token::EscapeStringLiteralContents(command);
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
        Token::IterateStringLiterals(str, [&tokens](string const &s, bool stringLiteral) {
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
      string str = command.substr(pos);
      Token::IterateStringLiterals(str, [&tokens](string const &s, bool stringLiteral) {
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
