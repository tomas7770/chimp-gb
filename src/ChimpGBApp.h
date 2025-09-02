#pragma once

#include <SDL.h>
#include <SDL_scancode.h>
#include "Gameboy.h"
#include "Config.h"

class ChimpGBApp
{
public:
    ChimpGBApp(const Cartridge &cart, std::string &romFilename, bool debug);
    void mainLoop();
    void drawDisplay();

private:
    constexpr const static char *WINDOW_TITLE = "ChimpGB - Game Boy Emulator";
    constexpr static int WINDOW_WIDTH = 640;
    constexpr static int WINDOW_HEIGHT = 576;

    constexpr static int AUDIO_SAMPLE_RATE = 44100;
    constexpr static uint16_t AUDIO_INTERNAL_BUFFER_SIZE = 1024;
    constexpr static int AUDIO_BUFFER_SIZE = 1024;
    constexpr static double CYCLES_PER_SAMPLE = double(Gameboy::CLOCK_RATE) / double(AUDIO_SAMPLE_RATE);

    constexpr const static char *SAVE_EXTENSION = ".sav";

    SDL_Window *mWindowSDL = NULL;
    SDL_Renderer *mRendererSDL = NULL;
    SDL_Texture *mTextureSDL = NULL;
    SDL_Event mEventSDL;
    SDL_AudioDeviceID mAudioDevSDL;
    uint32_t mTexturePixels[LCD::SCREEN_W * LCD::SCREEN_H];
    std::string mRomFilename;

    bool mFastForward = false;

    Gameboy *mGameboy = nullptr;

    Config mConfig;

    void createDataDirectories();
    void loadConfig();
    void saveConfig();
    void setFullscreen();
    void saveGame();
    void loadGame();
    void terminate(int error_code);
};
