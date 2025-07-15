#ifndef USEFUL
#define USEFUL

#include <string>
#include <vector>
#include <filesystem>

std::vector<std::string> split(std::string& s, const std::string& delimiter);
bool replace(std::string& str, const std::string& from, const std::string& to);
std::filesystem::path replace_home(std::filesystem::path& str);
std::filesystem::path revert_home(std::string& str);

#endif
