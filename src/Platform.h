// OS-specific code
#pragma once

#include <cstdint>
#include <string>

constexpr char const *CONFIG_NAME = "ChimpGB.ini";

void mainSleep(uint64_t ns);
std::string getSavesPath();
std::string getConfigsPath();
std::string getStatePath();
