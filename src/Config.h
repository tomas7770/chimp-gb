#pragma once

#include <string>

class Config
{
public:
    // keysGame
    int keysGame[8];

    // keysEmulator
    int keyFastForward;

    void load(std::stringstream &configString);
    void save(std::string &configFilepath);
};
