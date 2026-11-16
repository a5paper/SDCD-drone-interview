#ifndef DRONE_SIMULATOR_CONFIG_INI_FILE_H_
#define DRONE_SIMULATOR_CONFIG_INI_FILE_H_

#include <map>
#include <optional>
#include <string>

namespace drone_simulator::config {

using IniValues = std::map<std::string, std::string>;

std::optional<IniValues> ReadIniFile(const std::string& path);

}

#endif
