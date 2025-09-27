#include "useful.hpp"

std::vector<std::string> split(std::string& s, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t pos_start = 0, pos_end;
    std::string token;
    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delimiter.length();
        tokens.push_back(token);
    }
    tokens.push_back(s.substr(pos_start));

    return tokens;
}

bool replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

std::filesystem::path replace_home(std::filesystem::path str) {
  std::string temp = str.string();
  replace(temp, "~", std::getenv("HOME"));
  const std::filesystem::path temppath = temp;
  return temppath;
}

std::filesystem::path revert_home(std::string& str) {
  replace(str, std::getenv("HOME"), "~");
  const std::filesystem::path temppath = str;
  return temppath;
}
