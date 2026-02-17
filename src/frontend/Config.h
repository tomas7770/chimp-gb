#pragma once

#include <string>

class Config
{
public:
    enum AudioQuality
    {
        Low,
        High,
    };

    // keysGame
    int keysGame[8];

    // keysEmulator
    int keyShowMenuBar;
    int keyFastForward;
    int keyToggleFullscreen;

    // video
    int fullscreen;
    int exclusiveFullscreen;
    int integerScaling;
    float uiScale;

    // audio
    float audioVolume;
    int audioMute;
    int audioSampleRate;
    int audioBufferSize;
    int audioLatency;
    int audioQuality;

    // emulation
    std::string dmgBootRomPath;
    std::string cgbBootRomPath;
    int dmgGameEmulatedConsole;
    int cgbGameEmulatedConsole;

    void load(std::stringstream &configString);
    void save(std::string &configFilepath);
};
