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

    void load(std::stringstream &configString);
    void save(std::string &configFilepath);
};
