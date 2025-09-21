#pragma once

#include <string>

class Config
{
public:
    // keysGame
    int keysGame[8];

    // keysEmulator
    int keyFastForward;
    int keyToggleFullscreen;

    // video
    int fullscreen;

    // emulation
    std::string dmgBootRomPath;
    std::string cgbBootRomPath;

    void load(std::stringstream &configString);
    void save(std::string &configFilepath);
};
