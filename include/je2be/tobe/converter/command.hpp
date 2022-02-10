#pragma once

namespace je2be::tobe {

class Command {
public:
  static std::string Transpile(std::string const &inLine) {
    using namespace std;
    string line = inLine;
    string prefix;
    string suffix;
    while (true) {
      if (line.starts_with(" ")) {
        prefix += " ";
        line = line.substr(1);
      } else if (line.starts_with("/")) {
        prefix += "/";
        line = line.substr(1);
        break;
      } else {
        break;
      }
    }
    while (true) {
      if (line.ends_with("\x0d")) {
        suffix = "\x0d" + suffix;
        line = line.substr(0, line.size() - 1);
      } else if (line.ends_with(" ")) {
        suffix = " " + suffix;
        line = line.substr(0, line.size() - 1);
      } else {
        break;
      }
    }

    vector<string> tokens;
    vector<string> spaces;

    // 0: token
    // 1: quoted token
    // 2: space
    int state = 0;
    char prev = 0;
    string current;
    bool exit = false;
    for (size_t i = 0; i < line.size() && !exit; i++) {
      char ch = line[i];
      switch (state) {
      case 0: {
        if (prev == '\\') {
          current += ch;
        } else if (ch == '"') {
          state = 1;
          current += "\"";
        } else if (ch == ' ') {
          state = 2;
          tokens.push_back(current);
          current = " ";
        } else {
          current += ch;
        }
        break;
      }
      case 1: {
        if (prev == '\\') {
          current += ch;
        } else if (ch == '"') {
          state = 2;
          current += "\"";
          tokens.push_back(current);
          current = "";
        } else {
          current += ch;
        }
        break;
      }
      case 2: {
        if (ch == ' ') {
          current += " ";
        } else if (ch == '"') {
          state = 1;
          spaces.push_back(current);
          current = ch;
        } else if (ch == '#') {
          suffix = line.substr(i) + suffix;
          exit = true;
        } else {
          state = 0;
          spaces.push_back(current);
          current = ch;
        }
        break;
      }
      }
      prev = ch;
    }

    if (state == 0) {
      tokens.push_back(current);
    } else if (state == 1) {
      spaces.push_back(current);
    } else if (state == 2) {
      suffix = current + suffix;
    } else {
      return inLine;
    }

    if (tokens.size() != spaces.size() + 1) {
      return inLine;
    }

    for (int i = 1; i < tokens.size(); i++) {
      string prev = tokens[i - 1];
      string token = tokens[i];
      if (prev == "function") {
        auto found = token.find(':');
        if (found != string::npos) {
          token = token.substr(0, found) + "/" + token.substr(found + 1);
        }
      }
      tokens[i] = token;
    }

    string ret = prefix;
    for (int i = 0; i < tokens.size(); i++) {
      string token = tokens[i];
      ret += token;
      if (i < spaces.size()) {
        string space = spaces[i];
        ret += space;
      }
    }

    return ret + suffix;
  }

private:
  Command() = delete;
};

} // namespace je2be::tobe
