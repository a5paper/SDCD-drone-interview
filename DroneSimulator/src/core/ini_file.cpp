#include "core/ini_file.h"

#include <fstream>
#include <string>
#include <string_view>

namespace drone_simulator::config {
namespace {

std::string Trim(std::string_view text) {
  const size_t begin = text.find_first_not_of(" \t\r");
  if (begin == std::string_view::npos) {
    return "";
  }
  const size_t end = text.find_last_not_of(" \t\r");
  return std::string(text.substr(begin, end - begin + 1));
}

}

std::optional<IniValues> ReadIniFile(const std::string& path) {
  std::ifstream file(path);
  if (!file) {
    return std::nullopt;
  }

  IniValues values;
  std::string line;
  while (std::getline(file, line)) {
    const std::string trimmed = Trim(line);
    const size_t equals = trimmed.find('=');
    if (trimmed.empty() || trimmed[0] == ';' || trimmed[0] == '#' ||
        trimmed[0] == '[' || equals == std::string::npos) {
      continue;
    }
    values[Trim(trimmed.substr(0, equals))] = Trim(trimmed.substr(equals + 1));
  }
  return values;
}

}
